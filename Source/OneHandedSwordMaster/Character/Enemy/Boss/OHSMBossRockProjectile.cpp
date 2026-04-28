// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBossRockProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AOHSMBossRockProjectile::AOHSMBossRockProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// ─── 충돌 구체 ─────────────────────────────────────────────────────
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(30.f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	SetRootComponent(CollisionSphere);

	// ─── 바위 메시 ─────────────────────────────────────────────────────
	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
	RockMesh->SetupAttachment(CollisionSphere);
	RockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ─── 투사체 무브먼트 ───────────────────────────────────────────────
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(
		TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed       = 1000.f;
	ProjectileMovement->MaxSpeed           = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true; // 날아가는 방향으로 회전
	ProjectileMovement->ProjectileGravityScale   = 0.7f;
	ProjectileMovement->bShouldBounce      = false;

	// 자동 소멸 시간
	InitialLifeSpan = 8.f;
}

void AOHSMBossRockProjectile::BeginPlay()
{
	Super::BeginPlay();

	// OnHit 이벤트 바인딩
	CollisionSphere->OnComponentHit.AddDynamic(
		this, &AOHSMBossRockProjectile::OnHit);
}

void AOHSMBossRockProjectile::InitProjectile(AActor* InOwner,
	const FVector& InDirection,
	float InDamage, float InSpeed, float InGravityScale, float InImpactRadius)
{
	OwnerBoss    = InOwner;
	Damage       = InDamage;
	ImpactRadius = InImpactRadius;

	// 충돌 시 발사체 소유자 무시
	if (InOwner)
	{
		CollisionSphere->IgnoreActorWhenMoving(InOwner, true);
	}

	// 투사체 속도·중력 설정
	ProjectileMovement->InitialSpeed         = InSpeed;
	ProjectileMovement->MaxSpeed             = InSpeed * 1.5f;
	ProjectileMovement->ProjectileGravityScale = InGravityScale;
	ProjectileMovement->Velocity             = InDirection * InSpeed;
}

void AOHSMBossRockProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == OwnerBoss || OtherActor == this)
	{
		return;
	}

	HandleImpact(Hit.ImpactPoint, OtherActor);
}

void AOHSMBossRockProjectile::HandleImpact(const FVector& ImpactLocation, AActor* DirectHitActor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// ─── 데미지 적용 ───────────────────────────────────────────────────
	if (ImpactRadius > 0.f)
	{
		// AoE — 반경 내 모든 폰에 데미지
		TArray<AActor*> IgnoreActors;
		if (OwnerBoss) IgnoreActors.Add(OwnerBoss);
		IgnoreActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(
			World,
			Damage,
			ImpactLocation,
			ImpactRadius,
			nullptr,              // DamageType
			IgnoreActors,
			this,
			OwnerBoss ? OwnerBoss->GetInstigatorController() : nullptr,
			true                  // bDoFullDamage (감쇠 없음)
		);
	}
	else if (DirectHitActor)
	{
		// 단일 타겟
		UGameplayStatics::ApplyDamage(
			DirectHitActor,
			Damage,
			OwnerBoss ? OwnerBoss->GetInstigatorController() : nullptr,
			this,
			nullptr
		);
	}

	// ─── 이펙트 / 사운드 ───────────────────────────────────────────────
	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, ImpactEffect, ImpactLocation, FRotator::ZeroRotator);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, ImpactSound, ImpactLocation);
	}

	// 자가 소멸
	Destroy();
}
