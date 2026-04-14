// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMSlashProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"

AOHSMSlashProjectile::AOHSMSlashProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(60.f, 20.f, 20.f));
	// OverlapAll — 충돌로 막히지 않고 겹침 이벤트만 받음
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	RootComponent = CollisionBox;

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(RootComponent);
	NiagaraComp->SetAutoActivate(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed             = 1500.f;
	ProjectileMovement->MaxSpeed                 = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale   = 0.f; // 직선 비행
}

void AOHSMSlashProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AOHSMSlashProjectile::OnProjectileOverlap);

	SetLifeSpan(LifeTime);
}

void AOHSMSlashProjectile::InitProjectile(AActor* InOwner, float InDamage)
{
	OwnerActor = InOwner;
	Damage     = InDamage;

	if (ProjectileMovement)
	{
		// BeginPlay 이후에는 InitialSpeed 변경이 무의미 → Velocity 직접 설정
		ProjectileMovement->Velocity = GetActorForwardVector() * Speed;
		ProjectileMovement->MaxSpeed = Speed;
		ProjectileMovement->Activate(true);
	}
}

void AOHSMSlashProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor)) return;
	if (OtherActor == OwnerActor.Get()) return;
	if (OtherActor == this) return;

	// 히트 이펙트
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			HitEffect,
			GetActorLocation()
		);
	}

	// Enemy 태그가 없으면 무시
	if (!OtherActor->ActorHasTag(TEXT("Enemy"))) return;

	// OHSMHealthComponent로 직접 데미지 적용
	if (UOHSMHealthComponent* HealthComp = OtherActor->FindComponentByClass<UOHSMHealthComponent>())
	{
		HealthComp->TakeDamage(Damage, OwnerActor.Get());
	}

	Destroy();
}
