// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMUserWidget.h"
#include "OHSMManaBar.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMManaBar : public UOHSMUserWidget
{
	GENERATED_BODY()
	
public:
	UOHSMManaBar(const FObjectInitializer& OHSMManaBarInitializer);
	
protected:
	virtual void NativeConstruct() override;
	
public:
	void SetMaxMana(float NewMaxMana) { MaxMana = NewMaxMana; }
	
	UFUNCTION()
	void UpdateManaBar(float CurrentMana, float InMaxMana);
	
	UPROPERTY()
	TObjectPtr<class UProgressBar> ManaProgressBar;
	
protected:
	UPROPERTY()
	float MaxMana;
	
};
