// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMEnemyHpBarWidget.generated.h"

/**
 * 적 머리 위에 표시되는 체력바 위젯
 * - EnemyBase의 HealthComponent OnHealthChanged 델리게이트에 바인딩
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMEnemyHpBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// HealthComponent의 OnHealthChanged 시그니처와 동일
	UFUNCTION()
	void UpdateHpBar(float CurrentHealth, float MaxHealth, float Damage, AActor* DamageCauser);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> HpProgressBar;
};
