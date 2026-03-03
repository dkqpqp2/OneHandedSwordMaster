// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckLeash.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UBTService_CheckLeash : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CheckLeash();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Bloackboard")
	FBlackboardKeySelector IsLeashedKey;
	
	UPROPERTY(EditAnywhere, Category = "Bloackboard")
	FBlackboardKeySelector HomePosKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
	
};
