// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMInventoryTooltipWidget.generated.h"

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMInventoryTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemTypeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> SellPriceText;

public:
	void UpdateTooltip(const FItemData& ItemData);
};
