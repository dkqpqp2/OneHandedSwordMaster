// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_FindPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GamePlayStatics.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "DrawDebugHelpers.h"

UBTService_FindPlayer::UBTService_FindPlayer()
{
	NodeName = "FindPlayer";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	TargetActorKey.SelectedKeyName = FName(TEXT("TargetActor"));
}

void UBTService_FindPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* Controller = OwnerComp.GetAIOwner();
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(Controller->GetPawn());
	if (!IsValid(Enemy))
	{
		return;
	}
	
	FVector AILocation = Enemy->GetActorLocation();
	
	AILocation.Z -= Enemy->GetHalfHeight();
	
	FCollisionQueryParams param(NAME_None, false, Enemy);
	
	FHitResult result;
	
	bool IsCollision = GetWorld()->SweepSingleByChannel(result, AILocation, AILocation, 
		FQuat::Identity, ECC_GameTraceChannel3, FCollisionShape::MakeSphere(Enemy->DetectionRange), param);
	
#if ENABLE_DRAW_DEBUG
	
	FColor DrawColor = IsCollision ? FColor::Red : FColor::Green;
	
	DrawDebugSphere(GetWorld(), AILocation, Enemy->DetectionRange, 20, DrawColor, false, 0.35f);
	
#endif
	
	// Passive 타입은 피격 시에만 타겟 설정 — 이 서비스에서 덮어쓰지 않음
	if (Enemy->Personality == EEnemyPersonality::Passive)
	{
		return;
	}

	if (IsCollision)
	{
		// result.GetActor() : 충돌된 액터를 가져온다.
		Controller->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), result.GetActor());
	}
	else
	{
		Controller->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), nullptr);
	}
	
}
