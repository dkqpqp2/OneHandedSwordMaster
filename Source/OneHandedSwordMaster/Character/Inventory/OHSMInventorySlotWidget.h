// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMInventorySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemCount;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HoverHighlight;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Slot")
	int32 SlotIndex = -1;
	
	UPROPERTY(BlueprintReadOnly, Category = "Slot")
	FInventorySlot SlotData;
	
	UPROPERTY()
	TObjectPtr<class UOHSMInventoryComponent> InventoryComponent;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void InitializeSlot(int32 InSlotIndex, UOHSMInventoryComponent* InInventory);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void UpdateSlot(const FInventorySlot& NewSlotData);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void ClearSlot();
	
	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	int32 GetSlotIndex() const { return SlotIndex; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	bool IsEmpty() const { return SlotData.IsEmpty(); }
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void SetDragging(bool bIsDragging);
protected:
	const FItemData* GetItemData() const;
	
	void RefreshUI();
};
