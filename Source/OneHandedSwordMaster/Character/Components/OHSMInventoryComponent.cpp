// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMInventoryComponent.h"

UOHSMInventoryComponent::UOHSMInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UOHSMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Slots.SetNum(MaxSlotCount);
	
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		Slots[i].ItemID = FName(NAME_None);
		Slots[i].Count = 0;
	}
}

int32 UOHSMInventoryComponent::AddItem(FName ItemID, int32 Count)
{
	UE_LOG(LogTemp, Error, TEXT(""));
    UE_LOG(LogTemp, Error, TEXT("========================================"));
    UE_LOG(LogTemp, Error, TEXT("[Inventory] AddItem 시작"));
    UE_LOG(LogTemp, Error, TEXT("[Inventory] ItemID: %s"), *ItemID.ToString());
    UE_LOG(LogTemp, Error, TEXT("[Inventory] Count: %d"), Count);
    UE_LOG(LogTemp, Error, TEXT("========================================"));
    
    // ==================== 1. 유효성 체크 ====================
    if (ItemID.IsNone() || Count <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[Inventory] ❌ ItemID None 또는 Count <= 0"));
        return 0;
    }
    
    // ==================== 2. ItemDataTable 확인 ====================
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[Inventory] ❌❌❌ ItemDataTable nullptr!"));
        return 0;
    }
    
    UE_LOG(LogTemp, Display, TEXT("[Inventory] ✅ ItemDataTable: %s"), *ItemDataTable->GetName());
    
    // ==================== 3. ItemData 가져오기 ====================
    const FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("AddItem"));
    if (!ItemData)
    {
        UE_LOG(LogTemp, Error, TEXT("[Inventory] ❌❌❌ ItemData 없음! ItemID: %s"), *ItemID.ToString());
        UE_LOG(LogTemp, Error, TEXT("[Inventory] DataTable에 이 ItemID가 없어요!"));
        return 0;
    }
    
    UE_LOG(LogTemp, Display, TEXT("[Inventory] ✅ ItemData 찾음: %s"), *ItemData->ItemName.ToString());
    UE_LOG(LogTemp, Display, TEXT("[Inventory] MaxStackSize: %d"), ItemData->MaxStackSize);
    
    // ==================== 4. 슬롯 정보 ====================
    UE_LOG(LogTemp, Display, TEXT("[Inventory] 총 슬롯 개수: %d"), Slots.Num());
    
    int32 EmptyCount = 0;
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].ItemID.IsNone())
        {
            EmptyCount++;
        }
        else
        {
            UE_LOG(LogTemp, Display, TEXT("[Inventory] 슬롯 [%d]: %s x%d"), 
                i, *Slots[i].ItemID.ToString(), Slots[i].Count);
        }
    }
    
    UE_LOG(LogTemp, Display, TEXT("[Inventory] 빈 슬롯 개수: %d"), EmptyCount);
    
    // ==================== 5. 기존 슬롯에 추가 시도 ====================
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
            
            UE_LOG(LogTemp, Display, TEXT("[Inventory] 슬롯 [%d]에 추가: +%d (총: %d)"), 
                i, AddCount, Slot.Count);
        }
    }
    
    // ==================== 6. 새 슬롯에 추가 시도 ====================
    while (RemainingCount > 0)
    {
        int32 EmptySlot = FindEmptySlot();
        
        UE_LOG(LogTemp, Display, TEXT("[Inventory] FindEmptySlot 결과: %d"), EmptySlot);
        
        if (EmptySlot == -1)
        {
            UE_LOG(LogTemp, Error, TEXT("[Inventory] ❌ 빈 슬롯 없음! 남은 개수: %d"), RemainingCount);
            break;
        }
        
        int32 AddCount = FMath::Min(MaxStack, RemainingCount);
        
        Slots[EmptySlot].ItemID = ItemID;
        Slots[EmptySlot].Count = AddCount;
        RemainingCount -= AddCount;
        
        OnInventoryUpdated.Broadcast(EmptySlot);
        
        UE_LOG(LogTemp, Display, TEXT("[Inventory] 슬롯 [%d]에 새 아이템: %s x%d"), 
            EmptySlot, *ItemID.ToString(), AddCount);
    }
    
    // ==================== 7. 결과 ====================
    int32 ActuallyAdded = Count - RemainingCount;
    
    UE_LOG(LogTemp, Error, TEXT("[Inventory] 최종 결과: %d개 추가됨 (남은 개수: %d)"), 
        ActuallyAdded, RemainingCount);
    UE_LOG(LogTemp, Error, TEXT("========================================"));
    UE_LOG(LogTemp, Error, TEXT(""));
    
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
