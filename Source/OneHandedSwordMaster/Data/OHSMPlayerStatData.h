
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OHSMPlayerStatData.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatType : uint8 
{
	// 기본 스탯
	Level           UMETA(DisplayName = "레벨"),
	Experience      UMETA(DisplayName = "경험치"),
    
	// 전투 스탯
	Damage          UMETA(DisplayName = "공격력"),
	Defense         UMETA(DisplayName = "방어력"),
	CriticalChance  UMETA(DisplayName = "치명타 확률"),
	CriticalDamage  UMETA(DisplayName = "치명타 피해"),
    
	// 생존 스탯
	MaxHealth       UMETA(DisplayName = "최대 체력"),
	HealthRegen     UMETA(DisplayName = "체력 재생"),
	MaxMana         UMETA(DisplayName = "최대 마나"),
	ManaRegen       UMETA(DisplayName = "마나 재생"),
	MaxStamina      UMETA(DisplayName = "최대 스태미나"),
	StaminaRegen    UMETA(DisplayName = "스태미나 재생"),
    
	// 부가 스탯
	MoveSpeed       UMETA(DisplayName = "이동 속도"),
	AttackSpeed     UMETA(DisplayName = "공격 속도"),
	CooldownReduction UMETA(DisplayName = "쿨다운 감소")
};

USTRUCT(BlueprintType)
struct FPlayerStat
{
	GENERATED_BODY()
    
	/** 기본값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float BaseValue = 0.0f;
    
	/** 장비 보너스 */
	UPROPERTY(BlueprintReadWrite, Category = "Stat")
	float EquipmentBonus = 0.0f;
    
	/** 임시 버프/디버프 */
	UPROPERTY(BlueprintReadWrite, Category = "Stat")
	float TemporaryBonus = 0.0f;
    
	/** 최종값 계산 */
	float GetFinalValue() const
	{
		return BaseValue + EquipmentBonus + TemporaryBonus;
	}
    
	/** 퍼센트 보너스 적용 */
	float GetFinalValueWithPercent(float PercentBonus = 0.0f) const
	{
		float base = GetFinalValue();
		return base * (1.0f + PercentBonus / 100.0f);
	}
    
	FPlayerStat()
		: BaseValue(0.0f)
		, EquipmentBonus(0.0f)
		, TemporaryBonus(0.0f)
	{
	}
    
	FPlayerStat(float InBase)
		: BaseValue(InBase)
		, EquipmentBonus(0.0f)
		, TemporaryBonus(0.0f)
	{
	}
};

// ==================== 레벨별 스탯 테이블 ====================
USTRUCT(BlueprintType)
struct FPlayerStatTableRow : public FTableRowBase
{
	GENERATED_BODY()
    
	/** 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	int32 Level = 1;
    
	/** 필요 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	int32 RequiredExp = 100;
    
	/** 기본 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseHealth = 100.0f;
    
	/** 기본 마나 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseMana = 50.0f;
    
	/** 기본 스태미나 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseStamina = 100.0f;
    
	/** 기본 공격력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseAttack = 10.0f;
    
	/** 기본 방어력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDefense = 5.0f;
};
