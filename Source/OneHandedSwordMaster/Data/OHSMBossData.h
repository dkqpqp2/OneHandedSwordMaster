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

	/**
	 * 공격 유효 거리.
	 * - 근접 패턴 (Normal / Area / GroundSlam) : 최대 사거리
	 *     → 플레이어가 이 거리 이내일 때만 선택
	 * - 원거리 패턴 (Jump / Projectile)        : 최소 사거리
	 *     → 플레이어가 이 거리 이상일 때만 선택
	 * - 0 : 거리 제한 없음
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern",
		meta = (ClampMin = "0"))
	float AttackRange = 250.f;

	/**
	 * 원거리 패턴 전용 최대 사거리 (0 = 제한 없음).
	 * AttackRange ≤ 거리 ≤ MaxAttackRange 범위에서만 원거리 패턴 선택.
	 * 예) Jump: AttackRange=400, MaxAttackRange=900
	 *     → 너무 멀면 쫓아가고, 적당한 거리에서만 점프 공격 사용.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern",
		meta = (ClampMin = "0"))
	float MaxAttackRange = 0.f;

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
	/** 마지막 파동의 최대 반경 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|GroundSlam",
		meta = (ClampMin = "100",
			EditCondition = "AttackType == EBossAttackType::GroundSlam"))
	float ForwardSlamLength = 600.f;

	/**
	 * 부채꼴 반각도 (°). 전방 기준 좌우 각도.
	 * 60 → 총 120° 부채꼴 / 90 → 반원
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|GroundSlam",
		meta = (ClampMin = "10", ClampMax = "180",
			EditCondition = "AttackType == EBossAttackType::GroundSlam"))
	float ForwardSlamAngle = 60.f;

	/** 파동 횟수 (펑 펑 펑) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|GroundSlam",
		meta = (ClampMin = "1", ClampMax = "5",
			EditCondition = "AttackType == EBossAttackType::GroundSlam"))
	int32 SlamWaveCount = 3;

	/** 파동 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern|GroundSlam",
		meta = (ClampMin = "0.05",
			EditCondition = "AttackType == EBossAttackType::GroundSlam"))
	float SlamWaveInterval = 0.25f;

	FOHSMBossAttackPattern()
		: Phase(0), AttackType(EBossAttackType::Normal)
		, AttackMontage(nullptr)
		, Damage(40.f), AttackRange(250.f), MaxAttackRange(0.f), Weight(1.f), Cooldown(3.f)
		, AreaRadius(400.f)
		, ProjectileSpeed(1000.f), ProjectileGravityScale(0.7f), ProjectileImpactRadius(150.f)
		, JumpDuration(0.8f), JumpLandingRadius(350.f), JumpMinRange(400.f)
		, ForwardSlamLength(600.f), ForwardSlamAngle(60.f), SlamWaveCount(3), SlamWaveInterval(0.25f)
	{}
};
