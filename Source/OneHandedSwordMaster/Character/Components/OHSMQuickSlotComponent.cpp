// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMQuickSlotComponent.h"

#include "OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

UOHSMQuickSlotComponent::UOHSMQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	QuickSlots.Init(NAME_None, TotalSlotCount);
}

void UOHSMQuickSlotComponent::AssignItem(int32 SlotIndex, FName ItemID)
{
	if (!QuickSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	QuickSlots[SlotIndex] = ItemID;
	OnQuickSlotUpdated.Broadcast(SlotIndex, ItemID);
}

void UOHSMQuickSlotComponent::ClearSlot(int32 SlotIndex)
{
	AssignItem(SlotIndex, NAME_None);
}

void UOHSMQuickSlotComponent::UsePotionSlot(int32 PotionSlotIndex)
{
	if (!QuickSlots.IsValidIndex(PotionSlotIndex) || QuickSlots[PotionSlotIndex].IsNone())
	{
		return;
	}

	FName ItemID = QuickSlots[PotionSlotIndex];

	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (!Player)
	{
		return;
	}

	UOHSMInventoryComponent* InventoryComp = Player->GetInventoryComponent();
	if (!InventoryComp)
	{
		return;
	}

	int32 Removed = InventoryComp->RemoveItem(ItemID, 1);

	// 인벤토리에 아이템이 없으면 슬롯 자동 해제
	if (Removed <= 0 || InventoryComp->GetItemCount(ItemID) <= 0)
	{
		ClearSlot(PotionSlotIndex);
	}
}

FName UOHSMQuickSlotComponent::GetSlotItemID(int32 SlotIndex) const
{
	if (QuickSlots.IsValidIndex(SlotIndex))
	{
		return QuickSlots[SlotIndex];
	}
	return NAME_None;
}

bool UOHSMQuickSlotComponent::IsSlotEmpty(int32 SlotIndex) const
{
	return GetSlotItemID(SlotIndex).IsNone();
}
