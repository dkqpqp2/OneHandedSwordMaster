// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBTTask_BossAttack.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/Boss/OHSMBossBase.h"
#include "OneHandedSwordMaster/Data/OHSMBossData.h"

UOHSMBTTask_BossAttack::UOHSMBTTask_BossAttack()
{
	NodeName     = TEXT("Boss Attack");
	bNotifyTick  = true;

	// TargetActor 키 기본값 설정
	TargetActorKey.AddObjectFilter(this,
		GET_MEMBER_NAME_CHECKED(UOHSMBTTask_BossAttack, TargetActorKey),
		AActor::StaticClass());
}

EBTNodeResult::Type UOHSMBTTask_BossAttack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ElapsedTime    = 0.f;
	MontageDuration = 0.f;
	bUsedFallback  = false;

	AOHSMBossBase* Boss = Cast<AOHSMBossBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	// 패턴 선택
	const FOHSMBossAttackPattern* Pattern = Boss->SelectBossAttackPattern();

	if (!Pattern || !Pattern->AttackMontage)
	{
		// 현재 거리/쿨다운 조건에 맞는 패턴 없음 → Failed
		// BT Selector 가 MoveTo 로 폴백하여 보스가 플레이어에게 접근
		return EBTNodeResult::Failed;
	}

	// 공격 전 타겟 방향으로 즉시 회전
	Boss->FaceTarget();

	// 공격 상태 전환 + 몽타주 재생
	Boss->SetAIState(EEnemyAIState::Attacking);
	MontageDuration = Boss->PlayAnimMontage(Pattern->AttackMontage);

	if (MontageDuration <= 0.f)
	{
		// 몽타주 재생 실패 → 즉시 종료
		Boss->SetAIState(EEnemyAIState::Idle);
		return EBTNodeResult::Failed;
	}

	// 점프 패턴: 실제 대기 시간을 JumpDuration 기준으로 설정
	// → 몽타주 길이와 무관하게 점프가 끝날 때까지 Task 유지
	if (Pattern->AttackType == EBossAttackType::Jump && Pattern->JumpDuration > 0.f)
	{
		MontageDuration = FMath::Max(MontageDuration, Pattern->JumpDuration);
	}

	return EBTNodeResult::InProgress;
}

void UOHSMBTTask_BossAttack::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;

	if (ElapsedTime >= MontageDuration)
	{
		AOHSMBossBase* Boss = Cast<AOHSMBossBase>(OwnerComp.GetAIOwner()->GetPawn());

		// 점프 공격 중이면 착지 완료 후에 태스크 종료
		if (Boss && Boss->IsJumpAttacking())
		{
			return;
		}

		if (Boss && !bUsedFallback)
		{
			Boss->SetAIState(EEnemyAIState::Idle);
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UOHSMBTTask_BossAttack::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AOHSMBossBase* Boss = Cast<AOHSMBossBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (Boss)
	{
		Boss->StopAnimMontage();
		Boss->SetAIState(EEnemyAIState::Idle);
	}
	return Super::AbortTask(OwnerComp, NodeMemory);
}
