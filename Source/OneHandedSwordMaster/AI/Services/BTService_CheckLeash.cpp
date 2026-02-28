// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckLeash.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UBTService_CheckLeash::UBTService_CheckLeash()
{
	NodeName = "CheckLeash";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	IsLeashedKey.SelectedKeyName = FName(TEXT("IsLeashed"));
	HomeLocationKey.SelectedKeyName = FName(TEXT("HomeLocation"));
}

void UBTService_CheckLeash::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[CheckLeash] Blackboard 없음!"));
        return;
    }

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("[CheckLeash] AIController 없음!"));
        return;
    }

    AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(AIController->GetPawn());
    if (!Enemy)
    {
        UE_LOG(LogTemp, Error, TEXT("[CheckLeash] Enemy 없음!"));
        return;
    }
    
    if (!BlackboardComp->IsVectorValueSet(HomeLocationKey.SelectedKeyName))
    {
        FVector HomePos = Enemy->GetHomeLocation();
        BlackboardComp->SetValueAsVector(HomeLocationKey.SelectedKeyName, HomePos);
    }
    
    FVector CurrentPos = Enemy->GetActorLocation();
    FVector HomePos = Enemy->GetHomeLocation();
    float DistanceFromHome = FVector::Dist(CurrentPos, HomePos);
    
    float LeashRange = Enemy->LeashRange;
    
    bool bIsLeashed = BlackboardComp->GetValueAsBool(IsLeashedKey.SelectedKeyName);
    
    bool bOutOfRange = (DistanceFromHome > LeashRange);
    
    UE_LOG(LogTemp, Log, TEXT("[CheckLeash] 거리: %.0f / %.0f, 초과: %s, 리쉬 중: %s"), 
           DistanceFromHome, 
           LeashRange,
           bOutOfRange ? TEXT("YES") : TEXT("NO"),
           bIsLeashed ? TEXT("YES") : TEXT("NO"));
    
    if (bOutOfRange && !bIsLeashed)
    {
        UE_LOG(LogTemp, Error, TEXT("[CheckLeash] 리쉬 범위 초과! 복귀 시작"));
        
        BlackboardComp->SetValueAsBool(IsLeashedKey.SelectedKeyName, true);
        
        BlackboardComp->ClearValue(TEXT("TargetActor"));
        
        Enemy->ReturnToHome();
    }
    else if (!bOutOfRange && bIsLeashed)
    {
        // 홈 근처 도착 체크 (200 유닛 이내)
        if (DistanceFromHome <= 200.0f)
        {
            UE_LOG(LogTemp, Display, TEXT("[CheckLeash] 홈 복귀 완료! (거리: %.0f)"), DistanceFromHome);
            
            // 리쉬 해제
            BlackboardComp->SetValueAsBool(IsLeashedKey.SelectedKeyName, false);
            
            // 타겟 완전 제거
            BlackboardComp->ClearValue(TEXT("TargetActor"));
            Enemy->SetTarget(nullptr);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("[CheckLeash] 홈으로 이동 중... (거리: %.0f)"), DistanceFromHome);
        }
    }
}
