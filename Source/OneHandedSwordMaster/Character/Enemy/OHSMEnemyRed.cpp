// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyRed.h"

#include "OneHandedSwordMaster/AI/OHSMEnemyAIController.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAnimInstance.h"

AOHSMEnemyRed::AOHSMEnemyRed()
{
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RedMeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonMinions/Characters/Buff/Buff_Red/Meshes/Buff_Red.Buff_Red'"));
	
	if (RedMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(RedMeshAsset.Object);
	}
	
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimAsset(TEXT("/Script/Engine.AnimBlueprint'/Game/OneHandedSwordMaster/Animations/ABP_OHSMEnemy.ABP_OHSMEnemy_C'"));
	
	if (AnimAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimAsset.Class);
	}
	
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -90.0), FRotator(0.0, -90.0, 0.0));
	GetCapsuleComponent()->SetCapsuleHalfHeight(95.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);
	
	AIControllerClass = AOHSMEnemyAIController::StaticClass();
}

void AOHSMEnemyRed::ChangeAIAnimType(uint8 AnimType)
{
	Super::ChangeAIAnimType(AnimType);
	
	EnemyAnimInst->ChangeAnimType(static_cast<EEnemyAIState>(AnimType));
}

void AOHSMEnemyRed::BeginPlay()
{
	Super::BeginPlay();
	
	EnemyAnimInst = Cast<UOHSMEnemyAnimInstance>(GetMesh()->GetAnimInstance());
}

void AOHSMEnemyRed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
