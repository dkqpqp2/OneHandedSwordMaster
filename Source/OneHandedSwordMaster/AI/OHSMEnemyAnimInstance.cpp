// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyRed.h"

void UOHSMEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	AnimType = EEnemyAIState::Idle;
	
	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void UOHSMEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (Movement)
	{
		Velocity = Movement->Velocity;
		GroundSpeed = Velocity.Size2D();
		bIsIdle = GroundSpeed < MovingThreshould;
	}
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
