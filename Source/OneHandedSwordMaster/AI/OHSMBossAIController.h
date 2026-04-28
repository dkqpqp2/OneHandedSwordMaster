// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OHSMBossAIController.generated.h"

/**
 * 보스 전용 AIController.
 * BT_Boss / BB_Boss 를 사용하며 일반 몬스터와 독립적으로 동작.
 *
 * ■ 에디터에서 해야 할 일
 *   - BP_BossAIController를 만들고 BossBehaviorTree / BossBlackboard 지정
 *   - BB_Boss 키: TargetActor(Object), HomePos(Vector),
 *                 BossPhase(Int), bIsPhaseChanging(Bool)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMBossAIController : public AAIController
{
	GENERATED_BODY()

public:
	AOHSMBossAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** 보스 전용 Behavior Tree */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> BossBehaviorTree;

	/** 보스 전용 Blackboard */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<class UBlackboardData> BossBlackboard;

public:
	void RunAI();
	void StopAI();
};
