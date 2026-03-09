// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "OHSMHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UOHSMHUDWidget(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void NativeConstruct() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMHpBar> WidgetHpBar; 
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextHp;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMManaBar> WidgetManaBar; 
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMana;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMExpBar> WidgetExpBar; 
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextExp;
	
	void InitializeHUD();
	
public:
	UFUNCTION()
	void UpdateHp(float CurrentHp, float MaxHp);
    
	UFUNCTION()
	void UpdateMana(float CurrentMana, float MaxMana);  // ← 추가!
    
	UFUNCTION()
	void OnExpChanged(int32 CurrentExp, int32 RequiredExp);
};
