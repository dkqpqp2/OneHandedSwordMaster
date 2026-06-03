// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBTService_BossPhaseCheck.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/Boss/OHSMBossBase.h"

UOHSMBTService_BossPhaseCheck::UOHSMBTService_BossPhaseCheck()
{
	NodeName         = TEXT("Boss Phase Check");
	Interval         = 0.2f;
	RandomDeviation  = 0.05f;
}

void UOHSMBTService_BossPhaseCheck::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AOHSMBossBase* Boss        = Cast<AOHSMBossBase>(OwnerComp.GetAIOwner()->GetPawn());
	UBlackboardComponent* BB   = OwnerComp.GetBlackboardComponent();

	if (!Boss || !BB)
	{
		return;
	}

	BB->SetValueAsInt(TEXT("BossPhase"),             Boss->GetCurrentPhase());
	BB->SetValueAsBool(TEXT("bIsPhaseChanging"),     Boss->IsPhaseChanging());

	// 현재 거리·쿨다운·페이즈 조건을 모두 만족하는 공격 패턴 존재 여부
	// → BT 데코레이터가 이 값을 보고 BossChase를 끊고 BossAttack을 실행
	BB->SetValueAsBool(TEXT("bHasAvailablePattern"), Boss->HasAnyAvailablePattern());
}
