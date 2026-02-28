// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_EnemyAttack.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

class AOHSMEnemyBase;

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
	NodeName = "Enemy Attack";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		UObject* Target = BlackboardComp->GetValueAsObject(TEXT("TargetActor"));
		UE_LOG(LogTemp, Warning, TEXT("[BT Attack] Blackboard TargetActor: %s"), 
			   Target ? *Target->GetName() : TEXT("NULL"));
	}
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(AIController->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}
	
	if (!Enemy->IsInAttackRange())
	{
		return EBTNodeResult::Failed;
	}
	
	Enemy->PerformAttack();
	bIsAttacking = true;
	
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnemyAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(AIController->GetPawn());
	if (!Enemy)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	static float AttackTimer = 0.0f;
	
	if (bIsAttacking)
	{
		AttackTimer += DeltaSeconds;
		if (AttackTimer >= 3.0f)
		{
			bIsAttacking = false;
			AttackTimer = 0.0f;
			
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}
