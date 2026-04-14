// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCraftInfo.h"

#include "OHSMCraftMaterialItem.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCraftData.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"

void UOHSMCraftInfo::SetCraftItemData(const FOHSMCraftItemData& InData, FName InRecipeID)
{
	// InventoryComponent 조회 — Pawn 또는 PlayerController 어디서든 안전하게
	UOHSMInventoryComponent* InvComp = nullptr;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			InvComp = Pawn->FindComponentByClass<UOHSMInventoryComponent>();
		}
	}

	// 결과 아이템 정보 표시
	if (IsValid(InvComp))
	{
		const FItemData* ItemData = InvComp->GetItemData(InData.ResultItemID);
		if (ItemData)
		{
			if (Label_ResultName)
			{
				Label_ResultName->SetText(ItemData->ItemName);
			}
			if (Img_ResultItem && ItemData->ItemIcon)
			{
				Img_ResultItem->SetBrushFromTexture(ItemData->ItemIcon);
			}
		}
	}

	// 재료 목록
	if (!WrapBox_Material || !MaterialWidgetClass)
	{
		return;
	}

	WrapBox_Material->ClearChildren();

	for (const FCraftMaterialData& MaterialData : InData.Materials)
	{
		UOHSMCraftMaterialItem* MaterialWidget =
			CreateWidget<UOHSMCraftMaterialItem>(this, MaterialWidgetClass);
		if (IsValid(MaterialWidget))
		{
			MaterialWidget->SetMaterialData(MaterialData);
			WrapBox_Material->AddChild(MaterialWidget);
		}
	}
}

void UOHSMCraftInfo::ResetInfo()
{
	if (WrapBox_Material)
	{
		WrapBox_Material->ClearChildren();
	}

	if (Label_ResultName)
	{
		Label_ResultName->SetText(FText::GetEmpty());
	}

	if (Img_ResultItem)
	{
		Img_ResultItem->SetBrushFromTexture(nullptr);
	}
}

void UOHSMCraftInfo::RefreshMaterialCounts()
{
	if (!WrapBox_Material) return;

	for (UWidget* Child : WrapBox_Material->GetAllChildren())
	{
		if (UOHSMCraftMaterialItem* MaterialItem = Cast<UOHSMCraftMaterialItem>(Child))
		{
			MaterialItem->RefreshCount();
		}
	}
}
