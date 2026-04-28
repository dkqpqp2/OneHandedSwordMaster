// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBTTask_BossChase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/Boss/OHSMBossBase.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UOHSMBTTask_BossChase::UOHSMBTTask_BossChase()
{
	NodeName    = TEXT("Boss Chase");
	bNotifyTick = true;

	TargetActorKey.AddObjectFilter(this,
		GET_MEMBER_NAME_CHECKED(UOHSMBTTask_BossChase, TargetActorKey),
		AActor::StaticClass());
}

EBTNodeResult::Type UOHSMBTTask_BossChase::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CheckAccum = 0.f;

	AAIController* AIC  = OwnerComp.GetAIOwner();
	AOHSMBossBase* Boss = AIC ? Cast<AOHSMBossBase>(AIC->GetPawn()) : nullptr;
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(
		OwnerComp.GetBlackboardComponent()->GetValueAsObject(
			TargetActorKey.SelectedKeyName));

	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	// 이동 시작
	AIC->MoveToActor(Target, AcceptanceRadius);
	Boss->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Run));

	return EBTNodeResult::InProgress;
}

void UOHSMBTTask_BossChase::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CheckAccum += DeltaSeconds;
	if (CheckAccum < CheckInterval)
	{
		return; // 아직 체크 주기 아님
	}
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

	// ── 핵심 판단 ────────────────────────────────────────────────────────
	// 현재 거리·페이즈·쿨다운 조건을 모두 만족하는 패턴이 있으면
	// 즉시 추적을 중단하고 Succeeded → BT가 BossAttack 을 다시 시도.
	if (Boss->HasAnyAvailablePattern())
	{
		AIC->StopMovement();
		Boss->ChangeAIAnimType(static_cast<uint8>(EEnemyAIState::Idle));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 아직 사용 가능한 패턴 없음 → 이동 재요청
	// (플레이어가 도망가거나 NavMesh 재계산이 필요한 경우 경로 갱신)
	AIC->MoveToActor(Target, AcceptanceRadius);
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
