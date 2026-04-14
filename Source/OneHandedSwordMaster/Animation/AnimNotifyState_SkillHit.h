// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SkillHit.generated.h"

/**
 * 무기 Actor 없이 캐릭터 소켓 기반으로 히트 판정을 수행하는 NotifyState.
 * 땅긋기처럼 아이템 메시를 소켓에 직접 붙이는 방식에서 사용.
 *
 * 사용법:
 *  1. 공격 몽타주 타임라인에서 Add Notify State → AnimNotifyState_SkillHit
 *  2. 칼날이 지나가는 구간에 배치
 *  3. Details에서 DamageMultiplier, SweepRadius 등 조정
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UAnimNotifyState_SkillHit : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("SkillHit"); }

public:
	/** 공격력에 곱해지는 데미지 배율 (1.0 = 기본 공격력 그대로) */
	UPROPERTY(EditAnywhere, Category = "Hit", meta = (ClampMin = "0.1"))
	float DamageMultiplier = 1.5f;

	/** 히트 판정 구체 반지름 (cm) */
	UPROPERTY(EditAnywhere, Category = "Hit")
	float SweepRadius = 80.f;

	/** 캐릭터 전방 기준 히트 판정 시작 거리 (cm) */
	UPROPERTY(EditAnywhere, Category = "Hit")
	float ForwardOffset = 30.f;

	/** 히트 판정 거리 (ForwardOffset 에서 얼마나 더 앞까지 탐색) */
	UPROPERTY(EditAnywhere, Category = "Hit")
	float SweepDistance = 150.f;

	/** 히트 판정 콜리전 채널 */
	UPROPERTY(EditAnywhere, Category = "Hit")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

private:
	/** 이미 맞은 액터 — 구간 내 중복 히트 방지 */
	TSet<TWeakObjectPtr<AActor>> HitActors;

	void PerformSweep(USkeletalMeshComponent* MeshComp);
};
