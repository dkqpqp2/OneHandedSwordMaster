// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMUserWidget.h"
#include "OneHandedSwordMaster/Character/UI/OHSMUserWidget.h"
#include "OHSMHpBar.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMHpBar : public UOHSMUserWidget
{
	GENERATED_BODY()
	
public:
	UOHSMHpBar(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void NativeConstruct() override;
	
public:
	void SetMaxHp(float NewMaxHp) { MaxHp = NewMaxHp; }
	
	UFUNCTION()
	void UpdateHpBar(float CurrentHp, float InMaxHp);
	
	UPROPERTY()
	TObjectPtr<class UProgressBar> HpProgressBar;
protected:
	UPROPERTY()
	float MaxHp;

};
