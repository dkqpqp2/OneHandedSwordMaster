// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMWeaponBase.h"

#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"

AOHSMWeaponBase::AOHSMWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
	TraceStart = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TraceStart"));
	TraceStart->SetupAttachment(WeaponMesh);
	
	TraceStart->SetRelativeLocation(FVector(20.0f, 0.0f, 0.0f));
	
	TraceEnd = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TraceEnd"));
	TraceEnd->SetupAttachment(WeaponMesh);
	TraceEnd->SetRelativeLocation(FVector(120.0f, 0.0f, 0.0f));

	// 검 트레일 — 평소에는 비활성, 공격 타이밍에만 켜짐
	WeaponTrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponTrailComponent"));
	WeaponTrailComponent->SetupAttachment(WeaponMesh);
	WeaponTrailComponent->SetAutoActivate(false);
}

void AOHSMWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOHSMWeaponBase::EquipToCharacter(ACharacter* Character)
{
	if (!Character)
	{
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!CharacterMesh)
	{
		return;
	}
	
	AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		EquipSocketName
		);
}

void AOHSMWeaponBase::SheathToCharacter(ACharacter* Character)
{
	if (!Character)
	{
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!CharacterMesh)
	{
		return;
	}
	
	AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		SheathSocketName
	);
}

void AOHSMWeaponBase::ActivateTrail()
{
	if (!IsValid(WeaponTrailComponent))
	{
		return;
	}

	// 에셋이 아직 지정 안 됐으면 설정
	if (IsValid(WeaponTrailSystem) && WeaponTrailComponent->GetAsset() != WeaponTrailSystem)
	{
		WeaponTrailComponent->SetAsset(WeaponTrailSystem);
	}

	WeaponTrailComponent->Activate(true);
}

void AOHSMWeaponBase::DeactivateTrail()
{
	if (IsValid(WeaponTrailComponent))
	{
		WeaponTrailComponent->Deactivate();
	}
}

void AOHSMWeaponBase::StartTracing()
{
	bIsTracing = true;
	HitActors.Empty();
}

void AOHSMWeaponBase::StopTracing()
{
	bIsTracing = false;
}

void AOHSMWeaponBase::PerformTrace()
{
	if (!bIsTracing)
	{
		return;
	}
	
	FVector Start = TraceStart->GetComponentLocation();
	FVector End = TraceEnd->GetComponentLocation();
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	if (AActor* WeaponOwner = GetWeaponOwner())
	{
		QueryParams.AddIgnoredActor(WeaponOwner);
	}
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		QueryParams
	);
	
	#if ENABLE_DRAW_DEBUG
		FColor DrawColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), Start, End, DrawColor, false, 0.1f, 0, 2.0f);
	DrawDebugSphere(GetWorld(), Start, TraceRadius, 8, DrawColor, false, 0.1f);
	DrawDebugSphere(GetWorld(), End, TraceRadius, 8, DrawColor, false, 0.1f);
	#endif
	
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			
			if (!HitActor || HitActors.Contains(HitActor))
			{
				continue;
			}
			
			UE_LOG(LogTemp, Display, TEXT("[무기] Hit: %s"), *HitActor->GetName());
			UE_LOG(LogTemp, Display, TEXT("[무기] Tags: %d개"), HitActor->Tags.Num());
			
			bool bHasEnemyTag = false;
			
			for (const FName& Tag : HitActor->Tags)
			{
				UE_LOG(LogTemp, Display, TEXT("Tag: %s"), *Tag.ToString());
				if (Tag == FName(TEXT("Enemy")))
				{
					bHasEnemyTag = true;
				}
			}
			
			if (!HitActor->ActorHasTag(TEXT("Enemy")))
			{
				continue;
			}
			
			HitActors.Add(HitActor);
			OnHitDetected(HitActor, Hit);
		}
	}
}

void AOHSMWeaponBase::ClearHitActors()
{
	HitActors.Empty();
}

void AOHSMWeaponBase::OnHitDetected(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor)
	{
		return;
	}

	// 플레이어 스탯 기반 데미지 (장비 보너스 포함)
	float ActualDamage = BaseDamage;
	AActor* WeaponOwner = GetWeaponOwner();
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(WeaponOwner);
	if (Player)
	{
		UOHSMPlayerStatComponent* StatComp = Player->GetStatComponent();
		if (StatComp)
		{
			ActualDamage = StatComp->GetAttackPower();
		}
	}

	UE_LOG(LogTemp, Display, TEXT("[무기] ✅ 적 타격! %s (데미지: %.1f)"),
		   *HitActor->GetName(),
		   ActualDamage);

	if (UOHSMHealthComponent* HealthComp = HitActor->FindComponentByClass<UOHSMHealthComponent>())
	{
		HealthComp->TakeDamage(ActualDamage, Owner);
	}
	
	// 디버그 구체
	DrawDebugSphere(
		GetWorld(),
		HitResult.ImpactPoint,
		20.0f,
		12,
		FColor::Red,
		false,
		0.5f
	);
}

AActor* AOHSMWeaponBase::GetWeaponOwner() const
{
	AActor* Parent = GetAttachParentActor();
	
	return Parent ? Parent : GetOwner();
}

