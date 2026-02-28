// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyAnimInstance.h"

#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

void UOHSMEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	AnimType = EEnemyAIState::Idle;
}

void UOHSMEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}
