// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMEnemyProjectile.h"

#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AOHSMEnemyProjectile::AOHSMEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// ── 충돌 구체 ─────────────────────────────────────────────
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(20.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // Player 채널
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SetRootComponent(CollisionSphere);

	// ── Niagara (비주얼) ───────────────────────────────────────
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(CollisionSphere);
	NiagaraComp->SetAutoActivate(true);

	// ── 투사체 이동 ────────────────────────────────────────────
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // 중력 무시 — 직선 비행
}

void AOHSMEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AOHSMEnemyProjectile::OnProjectileOverlap);

	SetLifeSpan(LifeTime);
}

void AOHSMEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOHSMEnemyProjectile::InitProjectile(AActor* InOwner, const FVector& InDirection, float InDamage, float InSpeed)
{
	OwnerEnemy = InOwner;
	Damage     = InDamage;
	Speed      = InSpeed;

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed     = Speed;
		ProjectileMovement->Velocity     = InDirection.GetSafeNormal() * Speed;
	}
}

void AOHSMEnemyProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == OwnerEnemy)
	{
		return;
	}

	if (!OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	// 데미지 적용
	OtherActor->TakeDamage(
		Damage,
		FDamageEvent(),
		OwnerEnemy ? OwnerEnemy->GetInstigatorController() : nullptr,
		OwnerEnemy
	);

	// 히트 이펙트
	if (IsValid(HitEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			HitEffect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(0.3f)
		);
	}

	Destroy();
}
