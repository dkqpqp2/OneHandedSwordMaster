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

	// 장비 스탯 표시 
	if (StatText)
	{
		if (ItemData.ItemType == EItemType::Equipment)
		{
			FString StatStr;

			if (ItemData.BonusAtk > 0.0f)
			{
				StatStr += FString::Printf(TEXT("공격력  +%.0f\n"), ItemData.BonusAtk);
			}
			if (ItemData.BonusDef > 0.0f)
			{
				StatStr += FString::Printf(TEXT("방어력  +%.0f\n"), ItemData.BonusDef);
			}
			if (ItemData.BonusMaxHp > 0.0f)
			{
				StatStr += FString::Printf(TEXT("최대 HP +%.0f\n"), ItemData.BonusMaxHp);
			}
			if (ItemData.BonusMaxMp > 0.0f)
			{
				StatStr += FString::Printf(TEXT("최대 MP +%.0f\n"), ItemData.BonusMaxMp);
			}
			
			StatStr.TrimEndInline();

			if (!StatStr.IsEmpty())
			{
				StatText->SetText(FText::FromString(StatStr));
				StatText->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				StatText->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			StatText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
