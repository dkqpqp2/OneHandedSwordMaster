// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OneHandedSwordMaster/Data/OHSMPlayerStatData.h"
#include "OHSMPlayerStatComponent.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnHpZeroDelegate);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHpChangedDelegate, float /*CurrentHp*/, float /*MaxHp*/);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnManaChangedDelegate, float /*CurrentMana*/, float /*MaxMana*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, OldLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, int32, CurrentExp, int32, RequiredExp);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ONEHANDEDSWORDMASTER_API UOHSMPlayerStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOHSMPlayerStatComponent();
	
protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	
protected:
	/** 모든 스탯 저장 */
	UPROPERTY()
	TMap<EPlayerStatType, FPlayerStat> PlayerStats;

protected:
	/** 레벨별 스탯 DataTable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<UDataTable> PlayerStatTable;

public:
	FOnHpZeroDelegate OnHpZero;
	FOnHpChangedDelegate OnHpChanged;
	
	FOnManaChangedDelegate OnManaChanged;

	/** 레벨업 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelUp OnLevelUp;
    
	/** 경험치 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnExpChanged OnExpChanged;
	
	
	// ==================== HP 관리 ====================
public:
	/** 최대 HP (DataTable에서) */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHp() const;
    
	/** 현재 HP */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentHp() const { return CurrentHp; }
    
	/** 피해 적용 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float ApplyDamage(float InDamage, AActor* Attacker = nullptr);
    
	/** 회복 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Heal(float Amount);

protected:
	void SetHp(float NewHp, float Damage = 0.0f, AActor* Attacker = nullptr);
	
	// 디스크에 저장할 필요가 없는 데이터는 Transient 사용
	UPROPERTY(Transient ,VisibleInstanceOnly, Category = "Stat")
	float CurrentHp;
	
	// Mana 관리
public:
	UFUNCTION(BlueprintPure, Category = "Stat|Mana")
	float GetMaxMana() const { return GetStat(EPlayerStatType::MaxMana); }
	
	UFUNCTION(BlueprintPure, Category = "Stat|Mana")
	float GetCurrentMana() const { return CurrentMana; }
	
	UFUNCTION(BlueprintCallable, Category = "Stat|Mana")
	bool UseMana(float Amount);
	
	UFUNCTION(BlueprintCallable, Category = "Stat|Mana")
	void RestoreMana(float Amount);
	
protected:
	void SetMana(float NewMana);
    
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Stats|Mana")
	float CurrentMana;
		
public:
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddExperience(int32 Amount);
    
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void LevelUp();
    
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetLevel() const { return CurrentLevel; }
    
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetExpPercent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevel = 1;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentExp = 0;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 RequiredExp = 100;

	// ==================== 스탯 시스템 ====================
public:
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStat(EPlayerStatType StatType) const;
    
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetBaseStat(EPlayerStatType StatType, float Value);
    
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetAttackPower() const { return GetStat(EPlayerStatType::Damage); }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetDefensePower() const { return GetStat(EPlayerStatType::Defense); }

	/** 장비 보너스 추가 (스탯에 EquipmentBonus 누적) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Equipment")
	void AddEquipmentBonus(EPlayerStatType StatType, float Amount);

	/** 장비 보너스 제거 */
	UFUNCTION(BlueprintCallable, Category = "Stats|Equipment")
	void RemoveEquipmentBonus(EPlayerStatType StatType, float Amount);

protected:
	void InitializeStats();
	void ApplyLevelStats();
};
