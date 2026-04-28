// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "OHSMBTTask_BossPhaseChange.generated.h"

/**
 * 보스 페이즈 전환 Task.
 * BB의 bIsPhaseChanging == true 일 때 Decorator로 진입.
 *
 * 실행 흐름:
 *   1. Boss->GetCurrentPhase() 로 인덱스 계산
 *   2. PhaseChangeMontages[index] 재생
 *   3. 몽타주 종료 → FinishPhaseTransition() 호출
 *   4. BB bIsPhaseChanging = false 로 갱신
 *   5. Succeeded 반환
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBTTask_BossPhaseChange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UOHSMBTTask_BossPhaseChange();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;

private:
	float ElapsedTime    = 0.f;
	float MontageDuration = 0.f;
};
