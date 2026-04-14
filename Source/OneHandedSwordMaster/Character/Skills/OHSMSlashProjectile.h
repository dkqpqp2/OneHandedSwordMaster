// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OHSMSlashProjectile.generated.h"

/**
 * 참격 스킬 투사체 — 전방으로 날아가 적에게 데미지를 입힌다.
 * NiagaraComponent와 BoxComponent를 사용한다.
 * Blueprint에서 VFX와 메시를 설정한다.
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMSlashProjectile : public AActor
{
	GENERATED_BODY()

public:
	AOHSMSlashProjectile();

protected:
	virtual void BeginPlay() override;

public:
	/** 스폰 직후 호출 — 소유자와 데미지 초기화 */
	void InitProjectile(AActor* InOwner, float InDamage);

	// ─── 컴포넌트 ─────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UNiagaraComponent> NiagaraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	// ─── 투사체 설정 ─────────────────────────────────────────────
	/** 이동 속도 (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Speed = 1500.f;

	/** 스폰 후 자동 소멸까지 최대 수명 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float LifeTime = 3.f;

	/** 히트 시 재생할 Niagara 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<class UNiagaraSystem> HitEffect;

private:
	float Damage = 50.f;

	TWeakObjectPtr<AActor> OwnerActor;

	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
