// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnemyAttack.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UAnimNotify_EnemyAttack : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);
	
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bIsAreaAttack = false;
	
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (EditCondition  = "bIsAreaAttack"))
	float AttackRadius = 300.0f;
	
};
