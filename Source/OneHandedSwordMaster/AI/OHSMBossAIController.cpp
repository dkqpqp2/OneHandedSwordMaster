// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBossAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

AOHSMBossAIController::AOHSMBossAIController()
{
}

void AOHSMBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RunAI();
}

void AOHSMBossAIController::OnUnPossess()
{
	StopAI();
	Super::OnUnPossess();
}

void AOHSMBossAIController::RunAI()
{
	if (!BossBehaviorTree || !BossBlackboard)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[BossAI] BossBehaviorTree 또는 BossBlackboard가 설정되지 않았습니다."));
		return;
	}

	UBlackboardComponent* BB = nullptr;
	if (!UseBlackboard(BossBlackboard, BB))
	{
		return;
	}

	// 초기 BB 값 설정
	if (BB && GetPawn())
	{
		BB->SetValueAsVector(TEXT("HomePos"), GetPawn()->GetActorLocation());
		BB->SetValueAsInt(TEXT("BossPhase"), 1);
		BB->SetValueAsBool(TEXT("bIsPhaseChanging"), false);

		// 보스 맵 입장 즉시 플레이어를 타겟으로 설정
		// SetTarget() 사용 → BB 키 + C++ 멤버변수(TargetActor) 동시 업데이트
		if (AOHSMEnemyBase* Boss = Cast<AOHSMEnemyBase>(GetPawn()))
		{
			if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
			{
				Boss->SetTarget(Player);
				UE_LOG(LogTemp, Log, TEXT("[BossAI] 플레이어 타겟 자동 설정: %s"),
					*Player->GetName());
			}
			else
			{
				// 플레이어 로드 타이밍 문제 시 다음 프레임에서 재시도
				GetWorldTimerManager().SetTimerForNextTick([this]()
				{
					if (AOHSMEnemyBase* DelayedBoss = Cast<AOHSMEnemyBase>(GetPawn()))
					{
						if (APawn* DelayedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
						{
							DelayedBoss->SetTarget(DelayedPlayer);
							UE_LOG(LogTemp, Log, TEXT("[BossAI] 플레이어 타겟 지연 설정: %s"),
								*DelayedPlayer->GetName());
						}
					}
				});
			}
		}
	}

	RunBehaviorTree(BossBehaviorTree);
}

void AOHSMBossAIController::StopAI()
{
	if (UBehaviorTreeComponent* BTC = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTC->StopTree(EBTStopMode::Safe);
	}
}
