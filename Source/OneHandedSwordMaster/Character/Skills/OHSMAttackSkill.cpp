// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMAttackSkill.h"

#include "GameFramework/Character.h"

void UOHSMAttackSkill::ActivateSkill_Implementation(ACharacter* Instigator)
{
	if (!IsValid(Instigator) || !IsValid(SkillMontage))
	{
		return;
	}

	// 발동 이펙트 스폰
	SpawnActivateEffect(Instigator);

	// 몽타주 재생 + 몽타주가 끝날 때까지 이동 잠금
	PlayMontageAndBlockMovement(Instigator, SkillMontage, PlayRate);
}
