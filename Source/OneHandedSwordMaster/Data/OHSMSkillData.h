// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OHSMPlayerStatData.h"
#include "OHSMSkillData.generated.h"

class UOHSMSkillBase;

/** 스킬 종류 */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Passive	UMETA(DisplayName = "패시브"),
	Buff	UMETA(DisplayName = "버프"),
	Attack	UMETA(DisplayName = "타격"),
	Slash	UMETA(DisplayName = "참격")
};

/** 스킬 데이터 테이블 행 — DT_SkillData Row Name = SkillID */
USTRUCT(BlueprintType)
struct FOHSMSkillData : public FTableRowBase
{
	GENERATED_BODY()

	// ─── 기본 정보 ────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<UTexture2D> SkillIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	ESkillType SkillType = ESkillType::Attack;

	// ─── 사용 비용 ────────────────────────────────────────────────
	/** 스킬 사용 시 소모 마나 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0"))
	float ManaCost = 20.f;

	/** 쿨다운 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0"))
	float Cooldown = 5.f;

	// ─── 스킬 트리 ────────────────────────────────────────────────
	/** 습득에 필요한 스킬 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Tree", meta = (ClampMin = "1"))
	int32 SkillPointCost = 1;

	/** 이 스킬을 배우려면 먼저 배워야 할 스킬 IDs (DataTable Row Name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Tree")
	TArray<FName> PrerequisiteSkillIDs;

	/** 스킬 트리 행(Tier) — 0이 가장 낮은 단계 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Tree", meta = (ClampMin = "0"))
	int32 TreeTier = 0;

	/** 스킬 트리 열 — UI 수평 배치용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Tree", meta = (ClampMin = "0"))
	int32 TreeColumn = 0;

	// ─── 스킬 효과 수치 ──────────────────────────────────────────
	/**
	 * 스킬 주요 수치
	 * - 패시브 : 영구적으로 올릴 스탯 수치
	 * - 버프   : 일시적으로 올릴 스탯 수치
	 * - 타격   : 기본 공격력에 곱하는 데미지 배율 (1.0 = 100%)
	 * - 참격   : 투사체가 입히는 고정 데미지
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Effect")
	float EffectValue = 50.f;

	/** 지속 시간 (버프 스킬 전용 — 초, 패시브·타격·참격은 무시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Effect", meta = (ClampMin = "0"))
	float Duration = 10.f;

	/**
	 * 올릴 스탯 종류 (패시브·버프 스킬 전용)
	 * - 패시브 : 영구적으로 올릴 스탯
	 * - 버프   : 일시적으로 올릴 스탯
	 * - 타격·참격은 무시
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Effect")
	EPlayerStatType BuffStatType = EPlayerStatType::Damage;


	// ─── 콤보 해금 ───────────────────────────────────────────────
	/**
	 * 이 패시브 스킬을 배우면 해금되는 최대 콤보 타격 수.
	 * 0 이면 콤보와 무관한 패시브. 1 이상이면 배울 때 CombatComponent의
	 * MaxComboCount 를 이 값으로 설정 (더 큰 값일 때만 적용).
	 * 예) 2 → 2타 콤보 해금, 3 → 3타 콤보 해금
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Effect",
		meta = (ClampMin = "0", EditCondition = "SkillType == ESkillType::Passive"))
	int32 ComboCountUnlock = 0;

	// ─── 스킬 클래스 ─────────────────────────────────────────────
	/** 실제 실행 로직을 담은 UOHSMSkillBase 서브클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<UOHSMSkillBase> SkillClass;

	FOHSMSkillData()
		: ManaCost(20.f)
		, Cooldown(5.f)
		, SkillPointCost(1)
		, TreeTier(0)
		, TreeColumn(0)
		, EffectValue(50.f)
		, Duration(10.f)
	{}
};
