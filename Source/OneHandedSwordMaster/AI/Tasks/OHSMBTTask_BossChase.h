// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "OHSMBTTask_BossChase.generated.h"

/**
 * 보스 전용 추적(Chase) Task.
 *
 * ■ 동작 방식
 *   1. MoveToActor 로 플레이어를 향해 이동 시작.
 *   2. CheckInterval 마다 Boss->HasAnyAvailablePattern() 을 확인.
 *      - true  → 이동 중단 + Succeeded → BT가 BTTask_BossAttack 으로 돌아감.
 *      - false → 계속 추적 (이동 재요청으로 목표 위치 갱신).
 *
 * ■ 기존 OHSMBTTask_TraceTarget 과의 차이
 *   - TraceTarget : 플레이어가 AttackRange 안에 들어와야 끝남 (= 항상 근접거리까지 쫓아감).
 *   - BossChase   : 근거리·원거리 공격 패턴을 모두 고려하여 적절한 거리에서 멈춤.
 *                   쿨다운이 돌면 원거리에 있어도 즉시 공격 시도.
 *
 * ■ BT 배치 예시 (BT_Boss)
 *   Selector
 *   ├── BTTask_BossAttack     (실패 → 현재 패턴 없음)
 *   └── OHSMBTTask_BossChase  (패턴 생기면 Succeeded → 위로 올라가 재공격)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBTTask_BossChase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UOHSMBTTask_BossChase();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	/** TargetActor BB 키 */
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	/**
	 * 경로 탐색 수락 반경 (cm).
	 * 이 거리 이내로 들어오면 NavMesh가 "도착"으로 판단.
	 * 보스·플레이어 캡슐 반경 합보다 조금 크게 설정 권장 (기본 200).
	 */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "50"))
	float AcceptanceRadius = 200.f;

	/**
	 * 패턴 가용 여부 체크 주기 (초).
	 * 너무 짧으면 CPU 낭비, 너무 길면 반응이 느림. 0.15~0.25 권장.
	 */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.05"))
	float CheckInterval = 0.2f;

private:
	float CheckAccum = 0.f;
};
