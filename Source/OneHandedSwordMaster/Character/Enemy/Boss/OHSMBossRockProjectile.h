// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "OHSMBossRockProjectile.generated.h"

/**
 * 보스 바위 투사체.
 * - 포물선(Gravity) 궤적
 * - 착탄 시 ImpactRadius > 0 이면 AoE 데미지
 * - BP에서 RockMesh (Static Mesh), ImpactEffect (Niagara) 지정
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMBossRockProjectile : public AActor
{
	GENERATED_BODY()

public:
	AOHSMBossRockProjectile();

protected:
	virtual void BeginPlay() override;

	// ─── 컴포넌트 ──────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> RockMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	// ─── 이펙트 ────────────────────────────────────────────────────────
	/** 착탄 시 재생할 Niagara 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	/** 착탄 시 재생할 사운드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<USoundBase> ImpactSound;

	// ─── 전투 설정 ─────────────────────────────────────────────────────
	/** 단순 직렬 타겟 데미지 vs AoE 판정 */
	float Damage         = 40.f;
	float ImpactRadius   = 150.f;  // 0 이면 단일 타겟

	/** 스폰한 보스 (자신은 데미지 제외용) */
	UPROPERTY()
	TObjectPtr<AActor> OwnerBoss;

public:
	/**
	 * 투사체 초기화 — 보스의 LaunchSkillProjectile() 에서 호출.
	 * @param InOwner         스폰한 보스 액터
	 * @param InDirection     발사 방향 (정규화)
	 * @param InDamage        타격 데미지
	 * @param InSpeed         초기 속도 (cm/s)
	 * @param InGravityScale  중력 스케일 (포물선 강도)
	 * @param InImpactRadius  착탄 AoE 반경 (0 = 단일)
	 */
	void InitProjectile(AActor* InOwner, const FVector& InDirection,
		float InDamage, float InSpeed, float InGravityScale, float InImpactRadius);

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	/** 착탄 처리 — 데미지·이펙트·자가 소멸 */
	void HandleImpact(const FVector& ImpactLocation, AActor* DirectHitActor);
};
