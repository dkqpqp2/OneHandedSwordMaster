// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyRed.h"

#include "Components/SphereComponent.h"
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
	
	RSocketCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RFistCollision"));
	RSocketCollision->SetupAttachment(GetMesh(), TEXT("R_Socket")); 
	RSocketCollision->SetSphereRadius(25.0f);
	RSocketCollision->SetRelativeLocation(FVector(-15.0f, 0.0f, 0.0f));
	RSocketCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RSocketCollision->SetCollisionObjectType(ECC_WorldDynamic);
	RSocketCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RSocketCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

	// 왼손 콜리전
	LSocketCollision = CreateDefaultSubobject<USphereComponent>(TEXT("LFistCollision"));
	LSocketCollision->SetupAttachment(GetMesh(), TEXT("L_Socket"));
	LSocketCollision->SetSphereRadius(25.0f);
	LSocketCollision->SetRelativeLocation(FVector(15.0f, 0.0f, 0.0f));
	LSocketCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LSocketCollision->SetCollisionObjectType(ECC_WorldDynamic);
	LSocketCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	LSocketCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -90.0), FRotator(0.0, -90.0, 0.0));
	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);
	
	AIControllerClass = AOHSMEnemyAIController::StaticClass();
}

void AOHSMEnemyRed::ChangeAIAnimType(uint8 AnimType)
{
	Super::ChangeAIAnimType(AnimType);
}

void AOHSMEnemyRed::SetAttackCollisionEnabled(bool bEnable)
{
	SetRSocketCollisionEnabled(bEnable);
	SetLSocketCollisionEnabled(bEnable);
}

void AOHSMEnemyRed::SetRSocketCollisionEnabled(bool bEnable)
{
	if (!IsValid(RSocketCollision))
	{
		return;
	}
	RSocketCollision->SetCollisionEnabled(
		bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AOHSMEnemyRed::SetLSocketCollisionEnabled(bool bEnable)
{
	if (!IsValid(LSocketCollision))
	{
		return;
	}
	LSocketCollision->SetCollisionEnabled(
		bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AOHSMEnemyRed::OnCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor))
	{
		return;
	}
	
	if (OtherActor == this)
	{
		return;
	}
	
	if (!OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}
		
	PerformAttack(false, 0.0f, OtherActor);
}

void AOHSMEnemyRed::BeginPlay()
{
	Super::BeginPlay();
	
	EnemyAnimInst = Cast<UOHSMEnemyAnimInstance>(GetMesh()->GetAnimInstance());
	RSocketCollision->OnComponentBeginOverlap.AddDynamic(this, &AOHSMEnemyRed::OnCollisionOverlap);
	LSocketCollision->OnComponentBeginOverlap.AddDynamic(this, &AOHSMEnemyRed::OnCollisionOverlap);
	
#if WITH_EDITOR
	RSocketCollision->SetHiddenInGame(false);
	LSocketCollision->SetHiddenInGame(false);
#endif
	
}

void AOHSMEnemyRed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
