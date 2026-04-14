// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_SkillHit.h"

#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

void UAnimNotifyState_SkillHit::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	HitActors.Empty();
}

void UAnimNotifyState_SkillHit::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	PerformSweep(MeshComp);
}

void UAnimNotifyState_SkillHit::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Empty();
}

void UAnimNotifyState_SkillHit::PerformSweep(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;

	UWorld* World = Owner->GetWorld();
	if (!World) return;

	// 플레이어 스탯에서 공격력 가져오기
	float AttackPower = 50.f;
	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(Owner))
	{
		if (UOHSMPlayerStatComponent* StatComp = Player->GetStatComponent())
		{
			AttackPower = StatComp->GetAttackPower();
		}
	}
	const float ActualDamage = AttackPower * DamageMultiplier;

	// 스윕 위치 — 캐릭터 전방 ForwardOffset ~ ForwardOffset+SweepDistance
	const FVector Forward  = Owner->GetActorForwardVector();
	const FVector Origin   = Owner->GetActorLocation();
	const FVector Start    = Origin + Forward * ForwardOffset;
	const FVector End      = Origin + Forward * (ForwardOffset + SweepDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	TArray<FHitResult> Hits;
	World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(SweepRadius),
		Params
	);

#if ENABLE_DRAW_DEBUG
	DrawDebugSphere(World, Start, SweepRadius, 8, FColor::Orange, false, 0.1f);
	DrawDebugSphere(World, End,   SweepRadius, 8, FColor::Orange, false, 0.1f);
	DrawDebugLine(World, Start, End, FColor::Orange, false, 0.1f, 0, 2.f);
#endif

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor)) continue;
		if (!HitActor->ActorHasTag(TEXT("Enemy"))) continue;

		TWeakObjectPtr<AActor> WeakHit(HitActor);
		if (HitActors.Contains(WeakHit)) continue;

		HitActors.Add(WeakHit);

		if (UOHSMHealthComponent* HealthComp = HitActor->FindComponentByClass<UOHSMHealthComponent>())
		{
			HealthComp->TakeDamage(ActualDamage, Owner);
		}
	}
}
