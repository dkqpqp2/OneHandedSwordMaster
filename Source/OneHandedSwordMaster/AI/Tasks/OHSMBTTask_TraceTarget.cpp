// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMBTTask_TraceTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

// AI 이동 관련 라이브러리
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/PawnMovementComponent.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAnimInstance.h"

UOHSMBTTask_TraceTarget::UOHSMBTTask_TraceTarget()
{
	NodeName = TEXT("TraceTarget");
	
	bNotifyTick = true;
	
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UOHSMBTTask_TraceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	AAIController* Controller = OwnerComp.GetAIOwner();
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(Controller->GetPawn());
	if (!IsValid(Enemy))
	{
		return EBTNodeResult::Failed;
	}
	
	AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	// 타겟이 없으면
	if (!IsValid(TargetActor))
	{
		Controller->StopMovement();
		
		Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
		
		return EBTNodeResult::Failed;
	}
	
	// 장애물이 존재해도 피해서 찾아옴.
	UAIBlueprintHelperLibrary::SimpleMoveToActor(Controller, TargetActor);
	
	Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Run));
	
	return EBTNodeResult::InProgress;
}

void UOHSMBTTask_TraceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* Controller = OwnerComp.GetAIOwner();
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(Controller->GetPawn());
	if (!IsValid(Enemy))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		
		Controller->StopMovement();
		
		return;
	}
	
	AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	if (!IsValid(TargetActor))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		
		Controller->StopMovement();
		
		Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
		
		return;
	}
	
	FVector AILocation = Enemy->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();
	
	AILocation.Z = Enemy->GetHalfHeight();
	
	TargetLocation.Z -= Enemy->GetHalfHeight();
	
	// FVector::Distance : 두 위치 사이의 거리 구하는 함수.
	float Distance = FVector::Distance(AILocation, TargetLocation);
	
	Distance -= Enemy->GetCapsuleComponent()->GetScaledCapsuleRadius();
	
	UCapsuleComponent* TargetCapsule = Cast<UCapsuleComponent>(TargetActor->GetRootComponent());
	if (IsValid(TargetCapsule))
	{
		Distance -= TargetCapsule->GetScaledCapsuleRadius();
	}
	
	if (Distance <= 150.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		
		Controller->StopMovement();
		
		Enemy->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
	}
}

void UOHSMBTTask_TraceTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
}
