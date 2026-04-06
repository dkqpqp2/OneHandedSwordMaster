// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OHSMEnemyProjectile.generated.h"

/**
 * 적 스킬 투사체
 * - 스폰 시 지정된 방향으로 날아감
 * - 플레이어 충돌 시 데미지 적용 후 소멸
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	AOHSMEnemyProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** 투사체 초기화 — 스폰 직후 호출 */
	void InitProjectile(AActor* InOwner, const FVector& InDirection, float InDamage, float InSpeed);

	// ─── 컴포넌트 ───────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UNiagaraComponent> NiagaraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	// ─── 투사체 설정 ─────────────────────────────────────────────
	/** 발사 속도 (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Speed = 1200.0f;

	/** 데미지 (DT_AttackPattern에서 전달받음) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Damage = 30.0f;

	/** 스폰 후 자동 소멸 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float LifeTime = 5.0f;

	/** 히트 시 재생할 Niagara 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<class UNiagaraSystem> HitEffect;

private:
	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	TObjectPtr<AActor> OwnerEnemy;
};
