// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMAttackSkill.h"

#include "GameFramework/Character.h"

// 공격 스킬 발동
void UOHSMAttackSkill::ActivateSkill_Implementation(ACharacter* Instigator)
{
	if (!IsValid(Instigator) || !IsValid(SkillMontage))
	{
		return;
	}

	SpawnActivateEffect(Instigator); // 발동 이펙트 스폰

	PlayMontageAndBlockMovement(Instigator, SkillMontage, PlayRate); // 몽타주 재생
}
