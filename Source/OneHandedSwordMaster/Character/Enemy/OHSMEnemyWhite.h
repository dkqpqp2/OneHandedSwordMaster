// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMEnemyBase.h"
#include "OHSMEnemyWhite.generated.h"

/**
 * Passive(피격 반응형) 적 - 양손검 사용
 * - 플레이어에게 공격받아야 추격 시작
 * - LeashRange 벗어나면 HomePos로 복귀 (BTService_CheckLeash 처리)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMEnemyWhite : public AOHSMEnemyBase
{
	GENERATED_BODY()

public:
	AOHSMEnemyWhite();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void ChangeAIAnimType(uint8 AnimType) override;

	/** weapon_r 소켓 콜리전 On/Off (OnCollision / OffCollision Notify) */
	virtual void SetAttackCollisionEnabled(bool bEnable) override;

	/** 스킬 투사체 발사 (SpawnProjectile Notify) */
	virtual void LaunchSkillProjectile() override;

	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// ─── 무기 콜리전 ────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<class USphereComponent> WeaponCollision;

	// ─── 투사체 설정 ─────────────────────────────────────────────
	/** 발사할 투사체 Blueprint 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Projectile")
	TSubclassOf<class AOHSMEnemyProjectile> ProjectileClass;

	/** 투사체 발사 소켓 이름 (기본: Muzzle_Front_XForward) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Projectile")
	FName ProjectileSocketName = TEXT("Muzzle_Front_XForward");

	/** 투사체 속도 (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Projectile")
	float ProjectileSpeed = 1200.0f;

	/** 투사체 데미지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Projectile")
	float ProjectileDamage = 30.0f;

protected:
	TObjectPtr<class UOHSMEnemyAnimInstance> EnemyAnimInst;
};
