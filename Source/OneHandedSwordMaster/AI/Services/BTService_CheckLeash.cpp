// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckLeash.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UBTService_CheckLeash::UBTService_CheckLeash()
{
	NodeName = "CheckLeash";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	IsLeashedKey.SelectedKeyName = FName(TEXT("IsLeashed"));
	HomePosKey.SelectedKeyName = FName(TEXT("HomePos"));
	TargetActorKey.SelectedKeyName = FName(TEXT("TargetActor"));
}

void UBTService_CheckLeash::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(AIController->GetPawn());
	if (!Enemy)
	{
		return;
	}
	
	FVector CurrentPos = Enemy->GetActorLocation();
	FVector HomePos = Enemy->GetHomeLocation();
	float DistanceFromHome = FVector::Dist(CurrentPos, HomePos);
	float LeashRange = Enemy->LeashRange;
	
	bool bIsLeashed = BlackboardComp->GetValueAsBool(IsLeashedKey.SelectedKeyName);
	UObject* TargetActor = BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName);
	
	bool bOutOfRange = (DistanceFromHome > LeashRange);
	
	if (bOutOfRange && !bIsLeashed)
	{
		BlackboardComp->SetValueAsBool(IsLeashedKey.SelectedKeyName, true);
		if (TargetActor)
		{
			BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName);
		}
		Enemy->SetTarget(nullptr);
		Enemy->ReturnToHome();
		
		if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = Enemy->RunSpeed * 1.2f;
		}
	}
	else if (!bOutOfRange && bIsLeashed)
	{
		if (DistanceFromHome < 50.0f)
		{
			BlackboardComp->SetValueAsBool(IsLeashedKey.SelectedKeyName, false);
			
			BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName);
			Enemy->SetTarget(nullptr);
			
			if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
			{
				Movement->MaxWalkSpeed = Enemy->WalkSpeed;
			}
		}
	}
	
}
