// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBossAnimInstance.h"

#include "OneHandedSwordMaster/Character/Enemy/Boss/OHSMBossBase.h"

void UOHSMBossAnimInstance::AnimNotify_BossJump()
{
	if (AOHSMBossBase* Boss = Cast<AOHSMBossBase>(TryGetPawnOwner()))
	{
		Boss->TriggerJumpAttack();
	}
}

void UOHSMBossAnimInstance::AnimNotify_BossAreaAttack()
{
	if (AOHSMBossBase* Boss = Cast<AOHSMBossBase>(TryGetPawnOwner()))
	{
		Boss->TriggerAreaAttack();
	}
}

void UOHSMBossAnimInstance::AnimNotify_BossGroundSlamCharge()
{
	if (AOHSMBossBase* Boss = Cast<AOHSMBossBase>(TryGetPawnOwner()))
	{
		Boss->TriggerGroundSlamCharge();
	}
}

void UOHSMBossAnimInstance::AnimNotify_BossGroundSlam()
{
	if (AOHSMBossBase* Boss = Cast<AOHSMBossBase>(TryGetPawnOwner()))
	{
		Boss->TriggerGroundSlam();
	}
}
