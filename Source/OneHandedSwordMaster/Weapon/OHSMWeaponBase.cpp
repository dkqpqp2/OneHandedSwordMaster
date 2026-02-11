// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMWeaponBase.h"

#include "GameFramework/Character.h"

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
			if (!HitActor->ActorHasTag(TEXT("Enmey")))
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
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Hit %s !"), *HitActor->GetName()));
	}
}

AActor* AOHSMWeaponBase::GetWeaponOwner() const
{
	AActor* Parent = GetAttachParentActor();
	
	return Parent ? Parent : GetOwner();
}

