// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "OneHandedSwordMaster/Data/OHSMBossData.h"
#include "OHSMBTTask_BossAttack.generated.h"

/**
 * 보스 공격 Task.
 * SelectBossAttackPattern() 으로 현재 페이즈에 맞는 패턴을 선택하고
 * 해당 몽타주를 재생한다. 몽타주 종료 후 Succeeded 반환.
 *
 * 실제 데미지는 AnimNotify(OnCollision / BossAreaAttack / SpawnProjectile) 가 처리.
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBTTask_BossAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UOHSMBTTask_BossAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	/** Blackboard TargetActor 키 */
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	/** 패턴을 찾지 못했을 때 대기 시간 (초) — 이후 Succeeded 처리 */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.5"))
	float FallbackWaitTime = 1.5f;

private:
	float ElapsedTime   = 0.f;
	float MontageDuration = 0.f;
	bool  bUsedFallback = false;
};
