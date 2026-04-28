// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyBase.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"
#include "OneHandedSwordMaster/Character/UI/OHSMWidgetComponent.h"
#include "OneHandedSwordMaster/Character/UI/OHSMEnemyHpBarWidget.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"
#include "NiagaraFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DamageEvents.h"
#include "OneHandedSwordMaster/Item/OHSMPickupItem.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAnimInstance.h"

AOHSMEnemyBase::AOHSMEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캡슐이 카메라 SpringArm 트레이스를 막지 않도록 Camera 채널 무시
	// (기본 Capsule 프리셋은 ECC_Camera 를 Block → 플레이어 뒤에서 카메라가 당겨지는 문제)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	HealthComponent = CreateDefaultSubobject<UOHSMHealthComponent>(TEXT("HealthComponent"));

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(DetectionRange);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionSphere->SetGenerateOverlapEvents(true);

	// 머리 위 체력바 위젯 컴포넌트
	EnemyHpBar = CreateDefaultSubobject<UOHSMWidgetComponent>(TEXT("EnemyHpBar"));
	EnemyHpBar->SetupAttachment(GetMesh());
	EnemyHpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 220.0f));
	EnemyHpBar->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyHpBar->SetDrawSize(FVector2D(150.0f, 15.0f));
	EnemyHpBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	Tags.Add(FName(TEXT("Enemy")));
	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	

}

void AOHSMEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	HomeLocation = GetActorLocation();

	// ── 스폰 애니메이션 ───────────────────────────────────────────────────────
	if (SpawnAnimSequence)
	{
		// Spawning 상태로 전환 → ABP Blend Poses가 Spawning Pose로 이동
		SetAIState(EEnemyAIState::Spawning);

		// 애니메이션 재생 중 AI 로직 일시정지
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			if (AIC->BrainComponent)
			{
				AIC->BrainComponent->PauseLogic(TEXT("Spawning"));
			}
		}

		// 애니메이션 길이만큼 타이머 후 Idle 전환
		const float AnimLength = SpawnAnimSequence->GetPlayLength();
		if (AnimLength > 0.f)
		{
			FTimerHandle SpawnTimerHandle;
			GetWorldTimerManager().SetTimer(
				SpawnTimerHandle,
				this,
				&AOHSMEnemyBase::OnSpawnEnded,
				AnimLength,
				false
			);
		}
		else
		{
			OnSpawnEnded();
		}
	}
	
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

	// HP 바 위젯 바인딩
	if (EnemyHpBar)
	{
		EnemyHpBar->SetHiddenInGame(true);

		UOHSMEnemyHpBarWidget* HpWidget = Cast<UOHSMEnemyHpBarWidget>(EnemyHpBar->GetUserWidgetObject());
		if (HpWidget && HealthComponent)
		{
			// 초기 체력으로 바 설정
			HpWidget->UpdateHpBar(
				HealthComponent->GetCurrentHealth(),
				HealthComponent->GetMaxHealth(),
				0.0f,
				nullptr
			);
			// 체력 변경 시 자동 업데이트
			HealthComponent->OnHealthChanged.AddDynamic(HpWidget, &UOHSMEnemyHpBarWidget::UpdateHpBar);
		}
	}
}

void AOHSMEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 플레이어 거리 기준으로 체력바 표시/숨김
	if (EnemyHpBar && !bIsDead)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
			EnemyHpBar->SetHiddenInGame(Distance > HpBarDisplayRange);
		}
	}

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

	// Blackboard도 동기화 — Passive 타입은 BTService_FindPlayer를 건너뛰므로 직접 세팅
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsObject(TEXT("TargetActor"), NewTarget);
		}
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
	ChangeAIAnimType((uint8)NewState);

 	UE_LOG(LogTemp, Log, TEXT("[Enemy] %s - 상태 변경: %d"), *GetName(), (int32)NewState);
 
 	// 상태별 속도 조정
 	switch (CurrentState)
 	{
 		case EEnemyAIState::Idle:
 			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
 			break;
 		case EEnemyAIState::Spawning:
 			GetCharacterMovement()->MaxWalkSpeed = 0.0f;
 			break;
 		case EEnemyAIState::Patrol:
 			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
 			break;
 		case EEnemyAIState::Run:
 			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
 			break;
 		case EEnemyAIState::Attacking:
 			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
 			break;
 		case EEnemyAIState::Dead:
 			GetCharacterMovement()->MaxWalkSpeed = 0.0f;
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
	UOHSMEnemyAnimInstance* AnimInst = Cast<UOHSMEnemyAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->ChangeAnimType((EEnemyAIState)AnimType);
	}
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

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		AIController->BrainComponent->StopLogic(TEXT("Dead"));
	}

	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// DetectionSphere 비활성화
	if (DetectionSphere)
	{
		DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 체력바 숨기기
	if (EnemyHpBar)
	{
		EnemyHpBar->SetHiddenInGame(true);
	}
	
	DropItems();

	// 스포너에게 사망 알림
	OnEnemyDeathNotify.Broadcast(this);

	// 경험치 지급 + 퀘스트 알림
	AOHSMPlayerCharacter* PlayerChar = Cast<AOHSMPlayerCharacter>(Killer);
	if (IsValid(PlayerChar))
	{
		if (ExpReward > 0)
		{
			if (UOHSMPlayerStatComponent* StatComp = PlayerChar->GetStatComponent())
			{
				StatComp->AddExperience(ExpReward);
			}
		}

		// 퀘스트 처치 목표 진행도 업데이트
		if (!EnemyID.IsNone())
		{
			if (UOHSMQuestComponent* QuestComp = PlayerChar->GetQuestComponent())
			{
				QuestComp->NotifyEnemyKilled(EnemyID);
			}
		}
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

void AOHSMEnemyBase::OnSpawnEnded()
{
	// AI 로직 재개
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (AIC->BrainComponent)
		{
			AIC->BrainComponent->ResumeLogic(TEXT("Spawning"));
		}
	}

	// Idle 상태로 전환 → BT가 정상 동작 시작
	SetAIState(EEnemyAIState::Idle);
}

void AOHSMEnemyBase::DropItems()
{
	if (!DropItemTable)
	{
        return;
    }

    for (int32 i = 0; i < DropItem.Num(); i++)
    {
        const FDropItemData& DropItemData = DropItem[i];

        float RandomValue = FMath::FRand();
        
        if (RandomValue > DropItemData.DropChance)
        {
            continue;
        }

        TSubclassOf<AOHSMPickupItem> PickupClass = nullptr;
        
        if (TSubclassOf<AOHSMPickupItem>* FoundClass = PickupItemClasses.Find(DropItemData.ItemID))
        {
            PickupClass = *FoundClass;
        }
        else if (PickupItemClass)
        {
            PickupClass = PickupItemClass;

        }
        else
        {
            continue;
        }
    	
        int32 DropCount = FMath::RandRange(DropItemData.MinCount, DropItemData.MaxCount);
    	
        FVector RandomOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f
        );
        FVector SpawnLocation = GetActorLocation() + RandomOffset;

        // 지면 Line Trace로 정확한 높이 찾기
        FHitResult HitResult;
        FVector TraceStart = FVector(SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z + 200.0f);
        FVector TraceEnd   = FVector(SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z - 500.0f);
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            SpawnLocation.Z = HitResult.ImpactPoint.Z + 20.0f;
        }

        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        UE_LOG(LogTemp, Display, TEXT("[DropItems] [%d] 스폰 위치: (%.1f, %.1f, %.1f)"), 
            i, SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
    	
        AOHSMPickupItem* DroppedItem = GetWorld()->SpawnActorDeferred<AOHSMPickupItem>(
            PickupClass,
            FTransform(SpawnRotation, SpawnLocation),
            this,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );
        
        if (DroppedItem)
        {
            DroppedItem->InitializeItem(DropItemData.ItemID, DropCount, DropItemTable);
            DroppedItem->FinishSpawning(FTransform(SpawnRotation, SpawnLocation));
        }
        
    }
}


