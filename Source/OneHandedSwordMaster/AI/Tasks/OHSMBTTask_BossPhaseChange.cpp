// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBTTask_BossPhaseChange.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/Boss/OHSMBossBase.h"

UOHSMBTTask_BossPhaseChange::UOHSMBTTask_BossPhaseChange()
{
	NodeName    = TEXT("Boss Phase Change");
	bNotifyTick = true;
}

EBTNodeResult::Type UOHSMBTTask_BossPhaseChange::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ElapsedTime    = 0.f;
	MontageDuration = 0.f;

	AOHSMBossBase* Boss = Cast<AOHSMBossBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	Boss->SetAIState(EEnemyAIState::PhaseChange);

	// PhaseChangeMontages[0] = Phase1→2, [1] = Phase2→3
	const int32 MontageIndex = Boss->GetCurrentPhase() - 2; // Phase2→0, Phase3→1
	const auto& Montages     = Boss->GetPhaseChangeMontages();

	if (Montages.IsValidIndex(MontageIndex) && Montages[MontageIndex])
	{
		MontageDuration = Boss->PlayAnimMontage(Montages[MontageIndex]);
	}

	// 몽타주 없거나 재생 실패 → 즉시 완료
	if (MontageDuration <= 0.f)
	{
		Boss->FinishPhaseTransition();
		Boss->SetAIState(EEnemyAIState::Idle);
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(
			TEXT("bIsPhaseChanging"), false);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UOHSMBTTask_BossPhaseChange::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;

	if (ElapsedTime >= MontageDuration)
	{
		AOHSMBossBase* Boss = Cast<AOHSMBossBase>(OwnerComp.GetAIOwner()->GetPawn());
		if (Boss)
		{
			Boss->FinishPhaseTransition();
			Boss->SetAIState(EEnemyAIState::Idle);
		}

		OwnerComp.GetBlackboardComponent()->SetValueAsBool(
			TEXT("bIsPhaseChanging"), false);

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
