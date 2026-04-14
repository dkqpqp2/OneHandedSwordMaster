// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SkillTrail.generated.h"

/**
 * 스킬 몽타주 전용 트레일 NotifyState
 *
 * 사용법:
 *  1. 스킬 몽타주에서 칼날이 지나가는 구간에 Add Notify State → AnimNotifyState_SkillTrail
 *  2. 해당 Notify를 선택 후 Details에서 Trail System에 원하는 Niagara 에셋 지정
 *     → 스킬마다 다른 Trail System을 지정할 수 있음
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UAnimNotifyState_SkillTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("SkillTrail"); }

public:
	/** 몽타주의 이 Notify에서 사용할 트레일 Niagara 에셋 — 스킬마다 다르게 지정 */
	UPROPERTY(EditAnywhere, Category = "Trail")
	TObjectPtr<class UNiagaraSystem> TrailSystem;
};
