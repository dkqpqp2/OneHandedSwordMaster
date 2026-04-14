// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMSkillBase.h"
#include "OneHandedSwordMaster/Data/OHSMPlayerStatData.h"
#include "OHSMPassiveSkill.generated.h"

/**
 * 패시브 스킬 — 습득 즉시 지정 스탯을 영구적으로 올린다.
 * ActivateSkill은 호출되지 않는다 (마나/쿨다운 없음).
 *
 * EffectValue    : SkillData에서 설정 (올릴 스탯 수치)
 * PassiveStatType: 이 Blueprint에서 직접 설정 (어느 스탯을 올릴지)
 */
UCLASS(Blueprintable)
class ONEHANDEDSWORDMASTER_API UOHSMPassiveSkill : public UOHSMSkillBase
{
	GENERATED_BODY()

public:
	/** 어느 스탯을 영구적으로 올릴지 — Blueprint Default에서 설정 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Passive")
	EPlayerStatType PassiveStatType = EPlayerStatType::Damage;

	/** 습득 시 호출 — 영구 스탯 증가 적용 */
	virtual void OnLearned_Implementation(ACharacter* Instigator) override;
};
