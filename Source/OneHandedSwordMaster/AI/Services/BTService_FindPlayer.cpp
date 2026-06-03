// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_FindPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GamePlayStatics.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "Engine/OverlapResult.h"

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

	TArray<FOverlapResult> Overlaps;
	//
	GetWorld()->OverlapMultiByChannel(
		Overlaps, AILocation, FQuat::Identity,
		ECC_GameTraceChannel3,
		FCollisionShape::MakeSphere(Enemy->DetectionRange),
		param);

	AActor* FoundPlayer = nullptr;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		//
		if (IsValid(HitActor) && HitActor->ActorHasTag(TEXT("Player")))
		{
			FoundPlayer = HitActor;
			break;
		}
	}
	
	//
	if (Enemy->Personality == EEnemyPersonality::Passive)
	{
		return;
	}

	Controller->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), FoundPlayer);
	
}
