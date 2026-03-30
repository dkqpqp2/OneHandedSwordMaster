// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMQuickSlotComponent.h"

#include "OHSMInventoryComponent.h"
#include "OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"

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

	// 아이템 데이터에서 회복량 조회
	const FItemData* ItemData = InventoryComp->GetItemData(ItemID);
	if (!ItemData)
	{
		return;
	}

	int32 Removed = InventoryComp->RemoveItem(ItemID, 1);
	if (Removed <= 0)
	{
		return;
	}

	// HP / MP 회복 적용
	UOHSMPlayerStatComponent* StatComp = Cast<UOHSMPlayerStatComponent>(Player->GetStatComponent());
	if (StatComp)
	{
		if (ItemData->HealHpAmount > 0.0f)
		{
			StatComp->Heal(ItemData->HealHpAmount);
		}

		if (ItemData->HealMpAmount > 0.0f)
		{
			StatComp->RestoreMana(ItemData->HealMpAmount);
		}
	}

	// 인벤토리에 아이템이 더 없으면 슬롯 자동 해제
	if (InventoryComp->GetItemCount(ItemID) <= 0)
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

void UOHSMQuickSlotComponent::SwapSlots(int32 FromSlot, int32 ToSlot)
{
	if (!QuickSlots.IsValidIndex(FromSlot) || !QuickSlots.IsValidIndex(ToSlot))
	{
		return;
	}

	FName FromItem = QuickSlots[FromSlot];
	FName ToItem   = QuickSlots[ToSlot];

	QuickSlots[FromSlot] = ToItem;
	QuickSlots[ToSlot]   = FromItem;

	OnQuickSlotUpdated.Broadcast(FromSlot, QuickSlots[FromSlot]);
	OnQuickSlotUpdated.Broadcast(ToSlot,   QuickSlots[ToSlot]);
}

bool UOHSMQuickSlotComponent::IsSlotEmpty(int32 SlotIndex) const
{
	return GetSlotItemID(SlotIndex).IsNone();
}
