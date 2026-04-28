// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "OHSMBTService_BossPhaseCheck.generated.h"

/**
 * 보스 페이즈 모니터링 Service (0.2초 주기).
 * Boss의 현재 페이즈와 페이즈 전환 여부를 Blackboard에 지속적으로 반영한다.
 *
 * 갱신하는 BB 키:
 *   BossPhase      (Int)  — 현재 페이즈 (1 / 2 / 3)
 *   bIsPhaseChanging (Bool) — 전환 연출 중 여부
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBTService_BossPhaseCheck : public UBTService
{
	GENERATED_BODY()

public:
	UOHSMBTService_BossPhaseCheck();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;
};
