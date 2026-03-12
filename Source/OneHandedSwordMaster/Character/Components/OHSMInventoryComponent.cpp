// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMInventoryComponent.h"

UOHSMInventoryComponent::UOHSMInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UOHSMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Slots.SetNum(SlotCount);
}

int32 UOHSMInventoryComponent::AddItem(FName ItemID, int32 Count)
{
	if (ItemID.IsNone() || Count <= 0)
	{
		return 0;
	}
	
	const FItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		return 0;
	}
	
	int32 RemainingCount = Count;
	int32 MaxStack = ItemData->MaxStackSize;
	
	for (int32 i = 0; i < Slots.Num() && RemainingCount > 0; ++i)
	{
		FInventorySlot& Slot = Slots[i];
		
		if (Slot.ItemID == ItemID && Slot.Count < MaxStack)
		{
			int32 AvailableSpace = MaxStack - Slot.Count;
			int32 AddCount = FMath::Min(AvailableSpace, RemainingCount);
			
			Slot.Count += AddCount;
			RemainingCount -= AddCount;
			
			OnInventoryUpdated.Broadcast(i);
		}
	}
	
	while (RemainingCount > 0)
	{
		int32 EmptySlot = FindEmptySlot();
		
		if (EmptySlot == -1)
		{
			break;
		}
		
		int32 AddCount = FMath::Min(MaxStack, RemainingCount);
		
		Slots[EmptySlot].ItemID = ItemID;
		Slots[EmptySlot].Count= AddCount;
		RemainingCount -= AddCount;
		
		OnInventoryUpdated.Broadcast(EmptySlot);
	}
	
	int32 ActuallyAdded = Count - RemainingCount;
	
	return ActuallyAdded;
}

int32 UOHSMInventoryComponent::RemoveItem(FName ItemID, int32 Count)
{
	if (ItemID.IsNone() || Count <= 0)
	{
		return 0;
	}
	
	int32 RemainingCount = Count;
	
	for (int32 i = 0; i < Slots.Num() && RemainingCount > 0; ++i)
	{
		FInventorySlot& Slot = Slots[i];
		
		if (Slot.ItemID == ItemID)
		{
			int32 RemoveCount = FMath::Min(Slot.Count, RemainingCount);
			
			Slot.Count -= RemoveCount;
			RemainingCount -= RemoveCount;
			
			if (Slot.Count <= 0)
			{
				Slot.SlotClear();
			}
			
			OnInventoryUpdated.Broadcast(i);
		}
	}
	
	int32 ActuallyRemoved = Count - RemainingCount;
	
	return ActuallyRemoved;
}

bool UOHSMInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Count)
{
	if (!Slots.IsValidIndex(SlotIndex) || Count <= 0)
	{
		return false;
	}
	
	FInventorySlot& Slot = Slots[SlotIndex];
	
	if (Slot.IsEmpty())
	{
		return false;
	}
	
	int32 RemoveCount = FMath::Min(Slot.Count, Count);
	Slot.Count -= RemoveCount;
	
	if (Slot.Count <= 0)
	{
		Slot.SlotClear();
	}
	
	OnInventoryUpdated.Broadcast(SlotIndex);
	
	return true;
}

bool UOHSMInventoryComponent::SwapSlots(int32 FromIndex, int32 ToIndex)
{
	if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex))
	{
		return false;
	}
	
	if (FromIndex == ToIndex)
	{
		return false;
	}
	
	FInventorySlot Temp = Slots[FromIndex];
	Slots[FromIndex] = Slots[ToIndex];
	Slots[ToIndex] = Temp;
	
	OnInventoryUpdated.Broadcast(FromIndex);
	OnInventoryUpdated.Broadcast(ToIndex);
	
	return true;
}

int32 UOHSMInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 TotalCount = 0;
	
	for (const FInventorySlot& Slot : Slots)
	{
		if (Slot.ItemID == ItemID)
		{
			TotalCount += Slot.Count;
		}
	}
	
	return TotalCount;
}

int32 UOHSMInventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}
    
	return -1;
}

int32 UOHSMInventoryComponent::FindItemSlot(FName ItemID, bool bFindPartialStack) const
{
	const FItemData* ItemData = GetItemData(ItemID);
	
	if (!ItemData)
	{
		return -1;
	}
	
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		const FInventorySlot& Slot = Slots[i];
		if (Slot.ItemID == ItemID)
		{
			if (bFindPartialStack)
			{
				if (Slot.Count < ItemData->MaxStackSize)
				{
					return i;
				}
			}
			else
			{
				 return i;
			}
		}
	}
	return -1;
}

bool UOHSMInventoryComponent::SetSlotCount(int32 NewSlotCount)
{
	NewSlotCount = FMath::Clamp(NewSlotCount, MinSlotCount, MaxSlotCount);
	if (NewSlotCount == SlotCount)
	{
		return false;
	}
	
	if (NewSlotCount < SlotCount)
	{
		for (int32 i = NewSlotCount; i < Slots.Num(); ++i)
		{
			if (!Slots[i].IsEmpty())
			{
				return false;
			}
		}
	}

	Slots.SetNum(NewSlotCount);
	SlotCount = NewSlotCount;
		
	OnInventorySizeChanged.Broadcast(SlotCount);
	
	return true;
}

bool UOHSMInventoryComponent::AddSlots(int32 CountToAdd)
{
	return SetSlotCount(SlotCount + CountToAdd);
}

const FInventorySlot* UOHSMInventoryComponent::GetSlot(int32 Index) const
{
	if (Slots.IsValidIndex(Index))
	{
		return &Slots[Index];
	}
    
	return nullptr;
}

bool UOHSMInventoryComponent::IsSlotEmpty(int32 Index) const
{
	if (Slots.IsValidIndex(Index))
	{
		return Slots[Index].IsEmpty();
	}
    
	return true;
}

const FItemData* UOHSMInventoryComponent::GetItemData(FName ItemID) const
{
	if (!ItemDataTable)
	{
		return nullptr;
	}
	
	FString ContextString;
	const FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, ContextString);
    
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Inventory] 아이템 정보 없음: %s"), *ItemID.ToString());
	}
    
	return ItemData;
}

void UOHSMInventoryComponent::ClearInventory()
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (!Slots[i].IsEmpty())
		{
			Slots[i].SlotClear();
			OnInventoryUpdated.Broadcast(i);
		}
	}
}
