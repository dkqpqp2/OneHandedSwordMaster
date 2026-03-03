// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPatrolPos.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UBTTask_FindPatrolPos::UBTTask_FindPatrolPos()
{
}

EBTNodeResult::Type UBTTask_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)
	{
		return EBTNodeResult::Failed;
	}
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(ControllingPawn);
	if (!IsValid(Enemy))
	{
		return EBTNodeResult::Failed;
	}
	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(ControllingPawn->GetWorld());
	if (nullptr == NavSystem)
	{
		return EBTNodeResult::Failed;
	}
	
	FVector Origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("HomePos"));
	FNavLocation NextPatrolPos;
	
	if (NavSystem->GetRandomPointInNavigableRadius(Origin, 500.0f, NextPatrolPos))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPos"), NextPatrolPos.Location);
		Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Patrol));
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
