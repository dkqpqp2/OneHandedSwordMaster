// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SetIdleState.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAIController.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UBTTask_SetIdleState::UBTTask_SetIdleState()
{
	NodeName = "SetIdleState";
}

EBTNodeResult::Type UBTTask_SetIdleState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		return EBTNodeResult::Failed;
	}
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(AIController->GetPawn());
	if (!IsValid(Enemy))
	{
		return EBTNodeResult::Failed;
	}
	
	Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
	
	return EBTNodeResult::Succeeded;
}
