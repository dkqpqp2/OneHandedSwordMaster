// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

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
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(TryGetPawnOwner());
	if (!IsValid(Enemy))
	{
		return;
	}
	Enemy->SetAttackCollisionEnabled(true);
}

void UOHSMEnemyAnimInstance::AnimNotify_OffCollision()
{
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(TryGetPawnOwner());
	if (!IsValid(Enemy))
	{
		return;
	}
	Enemy->SetAttackCollisionEnabled(false);
}

void UOHSMEnemyAnimInstance::AnimNotify_SpawnProjectile()
{
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(TryGetPawnOwner());
	if (!IsValid(Enemy))
	{
		return;
	}
	Enemy->LaunchSkillProjectile();
}
