// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMInventoryTooltipWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOHSMInventoryTooltipWidget::UpdateTooltip(const FItemData& ItemData)
{
	if (ItemIcon)
	{
		if (ItemData.ItemIcon)
		{
			ItemIcon->SetBrushFromTexture(ItemData.ItemIcon);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(ItemData.ItemName);
	}

	if (ItemTypeText)
	{
		FText TypeText;
		switch (ItemData.ItemType)
		{
			case EItemType::Equipment:  
				TypeText = FText::FromString(TEXT("장비"));   
				break;
			case EItemType::Consumable: 
				TypeText = FText::FromString(TEXT("소비"));   
				break;
			case EItemType::Material:   
				TypeText = FText::FromString(TEXT("재료"));   
				break;
			case EItemType::Quest:      
				TypeText = FText::FromString(TEXT("퀘스트")); 
				break;
			default:                    
				TypeText = FText::GetEmpty();				
				break;
		}
		ItemTypeText->SetText(TypeText);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(ItemData.Description);
	}

	if (SellPriceText)
	{
		SellPriceText->SetText(FText::Format(
			FText::FromString(TEXT("판매가: {0} G")),
			FText::AsNumber(ItemData.SellPrice)
		));
	}
}
