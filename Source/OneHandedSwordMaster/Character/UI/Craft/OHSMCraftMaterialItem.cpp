// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCraftMaterialItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCraftData.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"

// 보유 수량 표시 공통 로직
static void ApplyCountDisplay(UTextBlock* Label, int32 HaveCount, int32 RequiredCount)
{
	if (!Label) return;

	const FString CountStr = FString::Printf(TEXT("%d / %d"), HaveCount, RequiredCount);
	Label->SetText(FText::FromString(CountStr));

	// 재료 부족 → 빨간색 / 충분 → 흰색
	const FSlateColor TextColor = (HaveCount >= RequiredCount)
		? FSlateColor(FLinearColor::White)
		: FSlateColor(FLinearColor(1.f, 0.2f, 0.2f, 1.f));
	Label->SetColorAndOpacity(TextColor);
}

void UOHSMCraftMaterialItem::SetMaterialData(const FCraftMaterialData& InMaterialData)
{
	// 다음 RefreshCount()를 위해 캐시
	CachedItemID        = InMaterialData.ItemID;
	CachedRequiredCount = InMaterialData.RequiredCount;

	UOHSMInventoryComponent* InvComp = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			InvComp = Pawn->FindComponentByClass<UOHSMInventoryComponent>();
		}
	}

	if (!IsValid(InvComp))
	{
		return;
	}

	const FItemData* ItemData = InvComp->GetItemData(InMaterialData.ItemID);
	if (!ItemData)
	{
		return;
	}

	if (Img_ItemIcon)
	{
		Img_ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
	}

	if (Label_ItemName)
	{
		Label_ItemName->SetText(ItemData->ItemName);
	}

	ApplyCountDisplay(Label_Count, InvComp->GetItemCount(CachedItemID), CachedRequiredCount);
}

void UOHSMCraftMaterialItem::RefreshCount()
{
	if (CachedItemID.IsNone()) return;

	UOHSMInventoryComponent* InvComp = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			InvComp = Pawn->FindComponentByClass<UOHSMInventoryComponent>();
		}
	}

	if (!IsValid(InvComp)) return;

	ApplyCountDisplay(Label_Count, InvComp->GetItemCount(CachedItemID), CachedRequiredCount);
}
