// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_SkillHit.h"

#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

// 스킬 히트 판정 시작
void UAnimNotifyState_SkillHit::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	HitActors.Empty(); // 피격 목록 초기화
}

// 매 틱 히트 판정
void UAnimNotifyState_SkillHit::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	PerformSweep(MeshComp); // 스윕 판정
}

// 스킬 히트 판정 종료
void UAnimNotifyState_SkillHit::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Empty(); // 피격 목록 정리
}

// 스윕으로 히트 판정 수행
void UAnimNotifyState_SkillHit::PerformSweep(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;

	UWorld* World = Owner->GetWorld();
	if (!World) return;

	// 공격력 + 스킬 배율로 최종 데미지 계산
	float AttackPower = 50.f;
	float SkillEffectValue = DamageMultiplier; // 기본 배율

	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(Owner))
	{
		if (UOHSMPlayerStatComponent* StatComp = Player->GetStatComponent())
		{
			AttackPower = StatComp->GetAttackPower();
		}
		// DataTable EffectValue (스킬 데미지 배율) 읽기
		if (UOHSMSkillComponent* SkillComp = Player->FindComponentByClass<UOHSMSkillComponent>())
		{
			SkillEffectValue = SkillComp->GetActiveSkillEffectValue();
		}
	}
	// 최종 데미지 = 플레이어 공격력 × DataTable EffectValue
	const float ActualDamage = AttackPower * SkillEffectValue;

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
