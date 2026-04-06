// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "OHSMEnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class ACharacter> Owner;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCharacterMovementComponent> Movement;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Velocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EEnemyAIState AnimType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GroundSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bIsIdle : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MovingThreshould;
	
public:
	void ChangeAnimType(EEnemyAIState Type)
	{
		AnimType = Type;
	}
	
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UFUNCTION()
	void AnimNotify_OnCollision();

	UFUNCTION()
	void AnimNotify_OffCollision();

	UFUNCTION()
	void AnimNotify_SpawnProjectile();
};
