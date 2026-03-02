// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyBase.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"
#include "NiagaraFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DamageEvents.h"

AOHSMEnemyBase::AOHSMEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	HealthComponent = CreateDefaultSubobject<UOHSMHealthComponent>(TEXT("HealthComponent"));
	
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(DetectionRange);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionSphere->SetGenerateOverlapEvents(true);
	
	Tags.Add(FName(TEXT("Enemy")));
	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	

}

void AOHSMEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	HomeLocation = GetActorLocation();
	
	if (DetectionSphere)
	{
		DetectionSphere->SetSphereRadius(DetectionRange);
		DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AOHSMEnemyBase::OnDetectionOverlap);
	}
	
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AOHSMEnemyBase::OnDeath);
		HealthComponent->OnHealthChanged.AddDynamic(this, &AOHSMEnemyBase::OnDamaged);
	}
	
}

void AOHSMEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if ENABLE_DRAW_DEBUG
	// 홈 위치에 구체 그리기
	DrawDebugSphere(
		GetWorld(),
		HomeLocation,
		50.0f,
		12,
		FColor::Green,
		false,
		0.1f
	);

	// 리쉬 범위 표시
	DrawDebugCircle(
		GetWorld(),
		HomeLocation,
		LeashRange,
		64,
		IsOutOfLeashRange() ? FColor::Red : FColor::Yellow,
		false,
		0.1f,
		0,
		2.0f,
		FVector(0, 1, 0),
		FVector(1, 0, 0)
	);

	// 현재 위치에서 홈까지 선
	if (TargetActor)
	{
		DrawDebugLine(
			GetWorld(),
			GetActorLocation(),
			HomeLocation,
			IsOutOfLeashRange() ? FColor::Red : FColor::Green,
			false,
			0.1f,
			0,
			2.0f
		);
	}
#endif
}

void AOHSMEnemyBase::SetTarget(AActor* NewTarget)
{
	TargetActor = NewTarget;
	
	if (TargetActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f,FColor::Red, *TargetActor->GetName());
	}
}

void AOHSMEnemyBase::PerformAttack(bool bIsAreaAttack, float Radius, AActor* HitTargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("PerformAttack 호출 - bIsAreaAttack: %d"), bIsAreaAttack);
    
	if (!CurrentAttackPattern)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentAttackPattern이 nullptr"));
		return;
	}

	TArray<AActor*> HitActors;

	if (bIsAreaAttack)
	{
		// 발밑 위치 기준
		FVector OriginLocation = GetActorLocation();
		OriginLocation.Z -= GetHalfHeight();

		// 범위 내 액터 감지
		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			OriginLocation,
			Radius,
			TArray<TEnumAsByte<EObjectTypeQuery>>{ ObjectTypeQuery3 }, // Pawn 채널
			nullptr,
			TArray<AActor*>{ this }, // 자기 자신 제외
			HitActors
		);

		// 이펙트 스폰
		if (IsValid(SkillHitEffect))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				SkillHitEffect,
				OriginLocation,
				FRotator::ZeroRotator,
				FVector(0.5,0.5,0.1)
			);
		}

#if WITH_EDITOR
		DrawDebugSphere(GetWorld(), OriginLocation, Radius, 16, FColor::Red, false, 1.0f);
#endif
	}
	else
	{
		// 단일 공격 - TargetActor에게만 데미지
		if (IsValid(HitTargetActor))
		{
			HitActors.Add(HitTargetActor);
			
			if (IsValid(AttackHitEffect))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					AttackHitEffect,
					HitTargetActor->GetActorLocation(), // 맞은 액터 위치
					FRotator::ZeroRotator,
					FVector(0.1f)
				);
			}
		}
	}

	// 데미지 적용
	for (AActor* HitActor : HitActors)
	{
		if (!HitActor->ActorHasTag(TEXT("Player"))) continue;

		HitActor->TakeDamage(
			CurrentAttackPattern->Damage,
			FDamageEvent(),
			GetController(),
			this
		);
		
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Damage : %f"), CurrentAttackPattern->Damage));
	}
}

bool AOHSMEnemyBase::IsInAttackRange() const
{
	if (!TargetActor)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	return Distance <= AttackRange;
}

void AOHSMEnemyBase::SetAIState(EEnemyAIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;

	UE_LOG(LogTemp, Log, TEXT("[Enemy] %s - 상태 변경: %d"), *GetName(), (int32)NewState);

	// 상태별 속도 조정
	switch (CurrentState)
	{
		case EEnemyAIState::Patrol:
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			break;
		case EEnemyAIState::Run:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;
		case EEnemyAIState::Attacking:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;
		default:
			break;
	}
}

bool AOHSMEnemyBase::IsOutOfLeashRange() const
{
	float DistanceFromHome = FVector::Dist(GetActorLocation(), HomeLocation);
	
	return DistanceFromHome > LeashRange;
}

void AOHSMEnemyBase::ReturnToHome()
{
	TargetActor = nullptr;
	
	SetAIState(EEnemyAIState::Patrol);
	
	if (bHealOnLeash && HealthComponent)
	{
		HealthComponent->Heal(HealthComponent->GetMaxHealth());
	}
}

struct FEnemyAttackPattern* AOHSMEnemyBase::SelectAttackPattern()
{
	if (!AttackPatternTable)
	{
		return nullptr;
	}

	TArray<TPair<FName, FEnemyAttackPattern*>> AvailablePatterns;
	float TotalWeight = 0.0f;
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (const FName& RowName : AttackPatternTable->GetRowNames())
	{
		FEnemyAttackPattern* Pattern = AttackPatternTable->FindRow<FEnemyAttackPattern>(RowName, TEXT(""));
		if (!Pattern)
		{
			continue;
		}
		
		// 쿨다운 체크
		if (Pattern->Cooldown > 0.0f)
		{
			float* LastUseTime = SkillCooldowns.Find(RowName);
			if (LastUseTime && (CurrentTime - *LastUseTime) < Pattern->Cooldown)
			{
				continue;
			}
		}

		AvailablePatterns.Add(TPair<FName, FEnemyAttackPattern*>(RowName, Pattern));
		TotalWeight += Pattern->Weight;
	}

	if (AvailablePatterns.IsEmpty())
	{
		return nullptr;
	}

	// 가중치 기반 랜덤 선택
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (auto& [RowName, Pattern] : AvailablePatterns)
	{
		CurrentWeight += Pattern->Weight;
		if (RandomValue <= CurrentWeight)
		{
			if (Pattern->Cooldown > 0.0f)
			{
				SkillCooldowns.Add(RowName, CurrentTime);
			}
			
			CurrentAttackPattern = Pattern;
			
			return CurrentAttackPattern;
		}
	}

	CurrentAttackPattern = AvailablePatterns[0].Value;
	// 만약을 위해
	return CurrentAttackPattern;
}

void AOHSMEnemyBase::ChangeAIAnimType(uint8 AnimType)
{
}

void AOHSMEnemyBase::OnDetectionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Aggressive 타입만 자동 감지
	if (Personality != EEnemyPersonality::Aggressive)
	{
		return;
	}

	// 플레이어 태그 체크
	if (OtherActor && OtherActor->ActorHasTag(TEXT("Player")))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Enemy] %s - 플레이어 감지! (Aggressive)"), *GetName());
		SetTarget(OtherActor);
		SetAIState(EEnemyAIState::Run);
	}
}

void AOHSMEnemyBase::OnDeath(AActor* Killer)
{
	UE_LOG(LogTemp, Error, TEXT("[Enemy] %s - 💀 사망!"), *GetName());

	SetAIState(EEnemyAIState::Dead);

	// 레그돌 활성화
	if (USkeletalMeshComponent* EnemyMesh = GetMesh())
	{
		EnemyMesh->SetSimulatePhysics(true);
		EnemyMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}

	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// DetectionSphere 비활성화
	if (DetectionSphere)
	{
		DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 5초 후 제거
	SetLifeSpan(5.0f);
}

void AOHSMEnemyBase::OnDamaged(float CurrentHealth, float MaxHealth, float Damage, AActor* DamageCauser)
{
	// Passive 타입: 맞아야 타겟 설정
	if (Personality == EEnemyPersonality::Passive && !TargetActor && DamageCauser)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Enemy] %s - 피격! 타겟 설정 (Passive)"), *GetName());
		SetTarget(DamageCauser);
		SetAIState(EEnemyAIState::Run);
	}
}


