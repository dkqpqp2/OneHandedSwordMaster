// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMWeaponBase.h"
#include "OHSMMeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMMeleeWeapon : public AOHSMWeaponBase
{
	GENERATED_BODY()
	
public:
	AOHSMMeleeWeapon();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combo")
	int32 MaxComboCount = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combo")
	TArray<float> ComboDamageMultipliers = {1.0f, 1.2f, 1.5f};
	
public:
	int32 GetMaxComboCount() const { return MaxComboCount; }
	
	UFUNCTION()
	float GetComboDamageMultiplier(int32 ComboIndex) const
	{
			return ComboDamageMultipliers.IsValidIndex(ComboIndex) ? ComboDamageMultipliers[ComboIndex] : 1.0f;
	}
};
