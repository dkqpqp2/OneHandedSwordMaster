// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckLeash.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UBTService_CheckLeash::UBTService_CheckLeash()
{
	NodeName = "CheckLeash";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	IsLeashedKey.SelectedKeyName = FName(TEXT("IsLeashed"));
	HomeLocationKey.SelectedKeyName = FName(TEXT("HomeLocation"));
}

void UBTService_CheckLeash::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
}
