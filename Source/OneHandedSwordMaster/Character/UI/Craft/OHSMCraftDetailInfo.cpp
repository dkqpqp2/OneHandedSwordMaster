// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCraftDetailInfo.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMCraftComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCraftData.h"

void UOHSMCraftDetailInfo::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Craft->OnClicked.AddDynamic(this, &UOHSMCraftDetailInfo::OnClickedCraft);

	// AutoWrapText는 한 번만 설정
	if (Label_Description)
	{
		Label_Description->SetAutoWrapText(true);
	}

	APawn* Pawn = GetOwningPlayerPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	CraftComp     = Pawn->FindComponentByClass<UOHSMCraftComponent>();
	InventoryComp = Pawn->FindComponentByClass<UOHSMInventoryComponent>();

	if (IsValid(InventoryComp))
	{
		InventoryComp->OnInventoryUpdated.AddDynamic(this, &UOHSMCraftDetailInfo::OnInventoryUpdated);
	}

}

void UOHSMCraftDetailInfo::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_Craft->OnClicked.RemoveAll(this);

	if (IsValid(InventoryComp))
	{
		InventoryComp->OnInventoryUpdated.RemoveAll(this);
	}

	CraftComp     = nullptr;
	InventoryComp = nullptr;
}

void UOHSMCraftDetailInfo::SetCraftItemData(const FOHSMCraftItemData& InData, FName InRecipeID)
{
	// Super 호출 전에 먼저 초기화 (WrapBox 누적 방지)
	ResetInfo();

	Super::SetCraftItemData(InData, InRecipeID);

	CurrentRecipeID = InRecipeID;

	if (Label_Description)
	{
		Label_Description->SetText(InData.Description);
	}

	UpdateCraftButton();
}

void UOHSMCraftDetailInfo::OnClickedCraft()
{
	if (CurrentRecipeID.IsNone()) return;

	// NativeConstruct 시점에 Pawn이 없었을 경우 재시도
	if (!IsValid(CraftComp))
	{
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			CraftComp = Pawn->FindComponentByClass<UOHSMCraftComponent>();
			}
	}

	if (!IsValid(CraftComp)) return;

	CraftComp->CraftItem(CurrentRecipeID);
}

void UOHSMCraftDetailInfo::OnInventoryUpdated(int32 SlotIndex)
{
	// 재료 보유 수량 표시 갱신
	RefreshMaterialCounts();

	// 제작 버튼 활성화 여부 갱신
	UpdateCraftButton();
}

void UOHSMCraftDetailInfo::UpdateCraftButton()
{
	if (!IsValid(Btn_Craft) || !IsValid(CraftComp) || CurrentRecipeID.IsNone())
	{
		return;
	}

	const bool bCanCraft = CraftComp->CanCraftItem(CurrentRecipeID);
	Btn_Craft->SetIsEnabled(bCanCraft);
}
