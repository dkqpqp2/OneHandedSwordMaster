// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyWhite.h"

#include "OHSMEnemyProjectile.h"
#include "Components/SphereComponent.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAIController.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAnimInstance.h"

AOHSMEnemyWhite::AOHSMEnemyWhite()
{
	PrimaryActorTick.bCanEverTick = true;

	Personality = EEnemyPersonality::Passive;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> WhiteMeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonMinions/Characters/Minions/White_Camp_Minion/Meshes/Minion_melee.Minion_melee'"));
	if (WhiteMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(WhiteMeshAsset.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimAsset(TEXT("/Game/OneHandedSwordMaster/Animations/ABP_OHSMEnemyWhite.ABP_OHSMEnemyWhite_C"));
	if (AnimAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimAsset.Class);
	}

	// ── weapon_r 소켓에 검 콜리전 ─────────────────────────────
	WeaponCollision = CreateDefaultSubobject<USphereComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(GetMesh(), TEXT("weapon_r"));
	WeaponCollision->SetRelativeLocationAndRotation(FVector(0.0f, -58.0f, 0.0f), FRotator(0.0f, 0.0f, -90.0f));
	WeaponCollision->SetSphereRadius(30.0f);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -55.0), FRotator(0.0, -90.0, 0.0));
	GetCapsuleComponent()->SetCapsuleHalfHeight(55.0f);
	GetCapsuleComponent()->SetCapsuleRadius(34.0f);

	AIControllerClass = AOHSMEnemyAIController::StaticClass();
}

void AOHSMEnemyWhite::BeginPlay()
{
	Super::BeginPlay();

	EnemyAnimInst = Cast<UOHSMEnemyAnimInstance>(GetMesh()->GetAnimInstance());

	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AOHSMEnemyWhite::OnWeaponOverlap);

#if WITH_EDITOR
	WeaponCollision->SetHiddenInGame(false);
#endif
}

void AOHSMEnemyWhite::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOHSMEnemyWhite::ChangeAIAnimType(uint8 AnimType)
{
	Super::ChangeAIAnimType(AnimType);
}

void AOHSMEnemyWhite::SetAttackCollisionEnabled(bool bEnable)
{
	if (!IsValid(WeaponCollision))
	{
		return;
	}
	WeaponCollision->SetCollisionEnabled(
		bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AOHSMEnemyWhite::LaunchSkillProjectile()
{
	if (!IsValid(ProjectileClass) || !IsValid(TargetActor))
	{
		return;
	}

	// 소켓 위치에서 스폰
	const FVector SpawnLocation = GetMesh()->GetSocketLocation(ProjectileSocketName);

	// 타겟(플레이어) 방향 계산 — 높이는 투사체 기준으로 맞춤
	FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();

	const FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner   = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AOHSMEnemyProjectile* Projectile = GetWorld()->SpawnActor<AOHSMEnemyProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (IsValid(Projectile))
	{
		Projectile->InitProjectile(this, Direction, ProjectileDamage, ProjectileSpeed);
	}
}

void AOHSMEnemyWhite::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}

	if (!OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	PerformAttack(false, 0.0f, OtherActor);
}
