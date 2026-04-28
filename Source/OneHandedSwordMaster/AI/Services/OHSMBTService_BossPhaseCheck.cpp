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

	BB->SetValueAsInt(TEXT("BossPhase"),        Boss->GetCurrentPhase());
	BB->SetValueAsBool(TEXT("bIsPhaseChanging"), Boss->IsPhaseChanging());
}
