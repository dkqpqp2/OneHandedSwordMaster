// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyAnimInstance.h"

#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyRed.h"

void UOHSMEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	AnimType = EEnemyAIState::Idle;
}

void UOHSMEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UOHSMEnemyAnimInstance::AnimNotify_OnCollision()
{
	AOHSMEnemyRed* Enemy = Cast<AOHSMEnemyRed>(TryGetPawnOwner());
	if (!IsValid(Enemy))
	{
		return;
	}
	Enemy->SetRSocketCollisionEnabled(true);
	Enemy->SetLSocketCollisionEnabled(true);
}

void UOHSMEnemyAnimInstance::AnimNotify_OffCollision()
{
	AOHSMEnemyRed* Enemy = Cast<AOHSMEnemyRed>(TryGetPawnOwner());
	if (!IsValid(Enemy))
	{
		return;
	}
	Enemy->SetRSocketCollisionEnabled(false);
	Enemy->SetLSocketCollisionEnabled(false);
}
