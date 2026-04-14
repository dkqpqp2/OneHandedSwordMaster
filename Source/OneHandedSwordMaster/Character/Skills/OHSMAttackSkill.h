// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMSkillBase.h"
#include "OHSMAttackSkill.generated.h"

/**
 * 타격 스킬 — AnimMontage를 재생하고 AnimNotify_WeaponTrace로 히트를 처리한다.
 * EffectValue : 데미지 배율 (0 이면 기본 공격력 그대로 사용)
 */
UCLASS(Blueprintable)
class ONEHANDEDSWORDMASTER_API UOHSMAttackSkill : public UOHSMSkillBase
{
	GENERATED_BODY()

public:
	/** 재생할 스킬 애니메이션 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Attack")
	TObjectPtr<UAnimMontage> SkillMontage;

	/** 재생 속도 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Attack", meta = (ClampMin = "0.1"))
	float PlayRate = 1.f;

	virtual void ActivateSkill_Implementation(ACharacter* Instigator) override;
};
