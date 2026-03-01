// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMEnemyBase.h"
#include "OHSMEnemyRed.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMEnemyRed : public AOHSMEnemyBase
{
	GENERATED_BODY()
	
public:
	AOHSMEnemyRed();
	
protected:
	TObjectPtr<class UOHSMEnemyAnimInstance> EnemyAnimInst;
	
public:
	virtual void ChangeAIAnimType(uint8 AnimType) override;
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
};
