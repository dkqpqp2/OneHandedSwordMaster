// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "Kismet/GameplayStatics.h"

AOHSMEnemyAIController::AOHSMEnemyAIController()
{
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> AITree(TEXT("/Script/AIModule.BehaviorTree'/Game/OneHandedSwordMaster/AI/BT_Enemy.BT_Enemy'"));
	if (AITree.Succeeded())
	{
		BehaviorTree = AITree.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UBlackboardData> Data(TEXT("/Script/AIModule.BlackboardData'/Game/OneHandedSwordMaster/AI/BB_Enemy.BB_Enemy'"));
	if (Data.Succeeded())
	{
		BlackboardAsset = Data.Object;
	}
}

void AOHSMEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BlackboardAsset)
	{
		UBlackboardComponent* BlackboardRef = Blackboard;
		
		if (UseBlackboard(BlackboardAsset, BlackboardRef))
		{
			RunBehaviorTree(BehaviorTree);
		}
	}
		
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(InPawn);
	if (!Enemy)
	{
		return;
	}
	
	// AI 실행
	RunAI();
}

void AOHSMEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();
	
	// AI 중지
	StopAI();
}

void AOHSMEnemyAIController::RunAI()
{
	if (!BehaviorTree)
	{
		UE_LOG(LogTemp, Error, TEXT("[AI Controller] Behavior Tree가 설정되지 않았습니다!"));
		return;
	}
	
	UBlackboardComponent* BlackboardComp = Blackboard;

	// Blackboard 초기화
	if (BlackboardAsset)
	{
		UseBlackboard(BlackboardAsset, BlackboardComp);
	}
	else if (BehaviorTree->BlackboardAsset)
	{
		UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);
	}

	// Behavior Tree 실행
	bool bSuccess = RunBehaviorTree(BehaviorTree);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("[AI Controller] Behavior Tree 실행 성공: %s"), *BehaviorTree->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AI Controller] Behavior Tree 실행 실패!"));
	}
}

void AOHSMEnemyAIController::StopAI()
{
	UBrainComponent* Brain = BrainComponent;
	if (Brain)
	{
		Brain->StopLogic(TEXT("AI Stopped"));
		UE_LOG(LogTemp, Log, TEXT("[AI Controller] Behavior Tree 중지"));
	}
}
