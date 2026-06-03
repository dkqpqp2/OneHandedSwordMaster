// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBTTask_BossChase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "OneHandedSwordMaster/Character/Enemy/Boss/OHSMBossBase.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UOHSMBTTask_BossChase::UOHSMBTTask_BossChase()
{
	NodeName    = TEXT("Boss Chase");
	bNotifyTick = true;

	TargetActorKey.AddObjectFilter(this,
		GET_MEMBER_NAME_CHECKED(UOHSMBTTask_BossChase, TargetActorKey),
		AActor::StaticClass());

	TargetActorKey.SelectedKeyName = FName(TEXT("TargetActor"));
}

EBTNodeResult::Type UOHSMBTTask_BossChase::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CheckAccum = 0.f;

	AAIController* AIC  = OwnerComp.GetAIOwner();
	AOHSMBossBase* Boss = AIC ? Cast<AOHSMBossBase>(AIC->GetPawn()) : nullptr;
	if (!Boss) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(
		OwnerComp.GetBlackboardComponent()->GetValueAsObject(
			TargetActorKey.SelectedKeyName));
	if (!Target) return EBTNodeResult::Failed;

	// TraceTarget 과 동일한 방식으로 이동 시작
	UAIBlueprintHelperLibrary::SimpleMoveToActor(AIC, Target);
	Boss->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Run));

	UE_LOG(LogTemp, Log, TEXT("[BossChase] 추적 시작 → %s"), *Target->GetName());

	return EBTNodeResult::InProgress;
}

void UOHSMBTTask_BossChase::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CheckAccum += DeltaSeconds;
	if (CheckAccum < CheckInterval) return;
	CheckAccum = 0.f;

	AAIController* AIC  = OwnerComp.GetAIOwner();
	AOHSMBossBase* Boss = AIC ? Cast<AOHSMBossBase>(AIC->GetPawn()) : nullptr;

	if (!Boss)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* Target = Cast<AActor>(
		OwnerComp.GetBlackboardComponent()->GetValueAsObject(
			TargetActorKey.SelectedKeyName));

	if (!Target)
	{
		AIC->StopMovement();
		Boss->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 플레이어가 움직이면 경로 재요청 (SimpleMoveToActor 는 재요청 비용이 낮음)
	UAIBlueprintHelperLibrary::SimpleMoveToActor(AIC, Target);
}

EBTNodeResult::Type UOHSMBTTask_BossChase::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC)
	{
		AIC->StopMovement();
		if (AOHSMBossBase* Boss = Cast<AOHSMBossBase>(AIC->GetPawn()))
		{
			Boss->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
		}
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}
