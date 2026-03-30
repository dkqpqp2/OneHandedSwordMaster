// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "Components/ActorComponent.h"
#include "OHSMInventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, int32, SlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventorySizeChanged, int32, NewSlotCount);

// 아이템 획득 알림용 (ItemID, 획득 개수)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, FName, ItemID, int32, Count);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ONEHANDEDSWORDMASTER_API UOHSMInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOHSMInventoryComponent();

protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventorySizeChanged OnInventorySizeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> Slots;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = "5", ClampMax = "100"))
	int32 SlotCount = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Size", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MinSlotCount = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Size", meta = (ClampMin = "10", ClampMax = "200"))
	int32 MaxSlotCount = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Data")
	TObjectPtr<UDataTable> ItemDataTable;
	
public:
	/** bSilent = true 이면 OnItemAdded 브로드캐스트 생략 (장비 해제 등 내부 이동 시 사용) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(FName ItemID, int32 Count = 1, bool bSilent = false);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItem(FName ItemID, int32 Count = 1);
		
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemFromSlot(int32 SlotIndex, int32 Count = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SwapSlots(int32 FromIndex, int32 ToIndex);
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 FindEmptySlot() const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 FindItemSlot(FName ItemID, bool bFindPartialStack = true) const;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Size")
	bool SetSlotCount(int32 NewSlotCount);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Size")
	bool AddSlots(int32 CountToAdd);
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Size")
	int32 GetSlotCount() const { return SlotCount; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Size")
	int32 GetMinSlotCount() const { return MinSlotCount; }
    
	UFUNCTION(BlueprintPure, Category = "Inventory|Size")
	int32 GetMaxSlotCount() const { return MaxSlotCount; }
	
public:
	const FInventorySlot* GetSlot(int32 Index) const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsSlotEmpty(int32 Index) const;
	
public:
	const FItemData* GetItemData(FName ItemID) const;

	UDataTable* GetItemDataTable() const { return ItemDataTable; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	void ClearInventory();
};
