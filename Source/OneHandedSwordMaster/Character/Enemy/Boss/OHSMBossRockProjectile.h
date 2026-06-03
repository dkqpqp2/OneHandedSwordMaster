// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "OHSMBossRockProjectile.generated.h"

// 보스 바위 투사체 (포물선 궤적, AoE 데미지)
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
	// 착탄 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	// 착탄 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<USoundBase> ImpactSound;

	// ─── 전투 설정 ─────────────────────────────────────────────────────
	// 데미지 / AoE 반경 설정
	float Damage         = 40.f;
	float ImpactRadius   = 150.f;  // 0 이면 단일 타겟

	// 스폰한 보스 (데미지 제외 대상)
	UPROPERTY()
	TObjectPtr<AActor> OwnerBoss;

public:
	// 투사체 초기화 (LaunchSkillProjectile 에서 호출)
	void InitProjectile(AActor* InOwner, const FVector& InDirection,
		float InDamage, float InSpeed, float InGravityScale, float InImpactRadius);

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	// 착탄 처리 (데미지·이펙트·소멸)
	void HandleImpact(const FVector& ImpactLocation, AActor* DirectHitActor);
};
