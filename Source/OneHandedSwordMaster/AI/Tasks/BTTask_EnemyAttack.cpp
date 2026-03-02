// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_EnemyAttack.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
	NodeName = "EnemyAttack";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	AAIController* Controller = OwnerComp.GetAIOwner();
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(Controller->GetPawn());
	if (!IsValid(Enemy))
	{
		return EBTNodeResult::Failed;
	}
	
	AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	if (!IsValid(TargetActor))
	{
		Controller->StopMovement();
		
		Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
		
		return EBTNodeResult::Failed;
	}
	
	FEnemyAttackPattern* Pattern = Enemy->SelectAttackPattern();
	if (!Pattern || !IsValid(Pattern->AttackMontage))
	{
		return EBTNodeResult::Failed;
	}
	
	MontageLength = Enemy->PlayAnimMontage(Pattern->AttackMontage);
	Enemy->SetTarget(TargetActor);
	ElapsedTime	 = 0.0f;
	
	if (MontageLength <= 0.0f)
	{
		return EBTNodeResult::Failed;
	}
	
	Enemy->SetAIState(EEnemyAIState::Attacking);
	Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Attacking));
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnemyAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	ElapsedTime += DeltaSeconds;
	
	if (ElapsedTime >= MontageLength)
	{
		AAIController* Controller = OwnerComp.GetAIOwner();
		AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(Controller->GetPawn());
		if (IsValid(Enemy))
		{
			Enemy->SetAIState(EEnemyAIState::Idle);
		}
		
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	
}

void UBTTask_EnemyAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	AAIController*  Controller = OwnerComp.GetAIOwner();
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(Controller->GetPawn());
	if (IsValid(Enemy))
	{
		Enemy->StopAnimMontage();
		Enemy->SetAIState(EEnemyAIState::Idle);
	}

	MontageLength = 0.0f;
	ElapsedTime   = 0.0f;
}
