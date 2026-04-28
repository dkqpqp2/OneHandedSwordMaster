// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OHSMBossData.generated.h"

/** 보스 공격 종류 */
UENUM(BlueprintType)
enum class EBossAttackType : uint8
{
	Normal		UMETA(DisplayName = "① 일반 펀치"),
	Projectile	UMETA(DisplayName = "② 바위 던지기"),
	Jump		UMETA(DisplayName = "③ 점프 공격"),
	Area		UMETA(DisplayName = "④ 에너지 방출"),
	GroundSlam	UMETA(DisplayName = "⑤ 전방 지면 강타"),
	Roar		UMETA(DisplayName = "포효 (버프/디버프)"),
	Summon		UMETA(DisplayName = "소환"),
};

/**
 * 보스 공격 패턴 DataTable 행.
 * Phase 0 = 전 페이즈 공통 / 1·2·3 = 특정 페이즈 전용.
 */
USTRUCT(BlueprintType)
struct FOHSMBossAttackPattern : public FTableRowBase
{
	GENERATED_BODY()

	// ─── 공통 ──────────────────────────────────────────────────────────
	/** 활성 페이즈 (0 = 전 페이즈 공통) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern",
		meta = (ClampMin = "0", ClampMax = "3"))
	int32 Phase = 0;

	/** 공격 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	EBossAttackType AttackType = EBossAttackType::Normal;

	/** 재생할 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 기본 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern",
		meta = (ClampMin = "0"))
	float Damage = 40.f;

	/** 공격이 유효한 거리 (BTTask에서 이 거리 이내일 때만 선택) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern",
		meta = (ClampMin = "0"))
	float AttackRange = 250.f;

	/** 패턴 선택 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern",
		meta = (ClampMin = "0.1"))
	float Weight = 1.f;

	/** 쿨다운 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern",
		meta = (ClampMin = "0"))
	float Cooldown = 3.f;

	// ─── ④ 에너지 방출 (Area) ──────────────────────────────────────────
	/** AoE 반경 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Area",
		meta = (ClampMin = "0", EditCondition = "AttackType == EBossAttackType::Area"))
	float AreaRadius = 500.f;

	// ─── ② 바위 던지기 (Projectile) ────────────────────────────────────
	/** 투사체 초기 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Projectile",
		meta = (ClampMin = "100",
			EditCondition = "AttackType == EBossAttackType::Projectile"))
	float ProjectileSpeed = 1000.f;

	/** 중력 스케일 (1.0 = 기본 중력, 포물선 궤적) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Projectile",
		meta = (ClampMin = "0",
			EditCondition = "AttackType == EBossAttackType::Projectile"))
	float ProjectileGravityScale = 0.7f;

	/** 착탄 시 AoE 반경 (0 = 단일 타겟) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Projectile",
		meta = (ClampMin = "0",
			EditCondition = "AttackType == EBossAttackType::Projectile"))
	float ProjectileImpactRadius = 150.f;

	// ─── ③ 점프 공격 (Jump) ────────────────────────────────────────────
	/** 점프 체공 시간 (초) — 길수록 높이/거리 증가 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Jump",
		meta = (ClampMin = "0.3",
			EditCondition = "AttackType == EBossAttackType::Jump"))
	float JumpDuration = 0.8f;

	/** 착지 폭발 반경 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Jump",
		meta = (ClampMin = "0",
			EditCondition = "AttackType == EBossAttackType::Jump"))
	float JumpLandingRadius = 350.f;

	/** 점프 발동 최소 거리 (이 거리 이상일 때만 선택) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|Jump",
		meta = (ClampMin = "0",
			EditCondition = "AttackType == EBossAttackType::Jump"))
	float JumpMinRange = 400.f;

	// ─── ⑤ 전방 지면 강타 (GroundSlam) ────────────────────────────────
	/** 강타 전방 길이 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|GroundSlam",
		meta = (ClampMin = "100",
			EditCondition = "AttackType == EBossAttackType::GroundSlam"))
	float ForwardSlamLength = 500.f;

	/** 강타 좌우 폭 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|GroundSlam",
		meta = (ClampMin = "50",
			EditCondition = "AttackType == EBossAttackType::GroundSlam"))
	float ForwardSlamWidth = 250.f;

	FOHSMBossAttackPattern()
		: Phase(0), AttackType(EBossAttackType::Normal)
		, AttackMontage(nullptr)
		, Damage(40.f), AttackRange(250.f), Weight(1.f), Cooldown(3.f)
		, AreaRadius(400.f)
		, ProjectileSpeed(1000.f), ProjectileGravityScale(0.7f), ProjectileImpactRadius(150.f)
		, JumpDuration(0.8f), JumpLandingRadius(350.f), JumpMinRange(400.f)
		, ForwardSlamLength(500.f), ForwardSlamWidth(250.f)
	{}
};
