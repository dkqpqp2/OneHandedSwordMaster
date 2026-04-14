// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPassiveSkill.h"

#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"

void UOHSMPassiveSkill::OnLearned_Implementation(ACharacter* Instigator)
{
	if (!IsValid(Instigator) || !SkillData) return;

	UOHSMPlayerStatComponent* StatComp =
		Instigator->FindComponentByClass<UOHSMPlayerStatComponent>();
	if (!IsValid(StatComp)) return;

	// 영구 보너스 — EquipmentBonus 슬롯에 추가 (장비처럼 영구 적용)
	StatComp->AddEquipmentBonus(PassiveStatType, SkillData->EffectValue);
}
