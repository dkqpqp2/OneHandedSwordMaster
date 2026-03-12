// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMInventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"

void UOHSMInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshUI();
}

FReply UOHSMInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InEvent);
	
	if (InEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (!SlotData.IsEmpty())
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}
	return FReply::Handled();
}

void UOHSMInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UDragDropOperation* DragDropOp = NewObject<UDragDropOperation>();
	
	if (DragDropOp)
	{
		DragDropOp->Payload = this;
		
		DragDropOp->DefaultDragVisual = this;
		
		OutOperation = DragDropOp;
	}
}

bool UOHSMInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	if (!InOperation || !InventoryComponent)
	{
		return false;
	}
	
	UOHSMInventorySlotWidget* DraggedSlot = Cast<UOHSMInventorySlotWidget>(InOperation->Payload);
	if (!DraggedSlot)
	{
		return false;
	}

	int32 FromIndex = DraggedSlot->GetSlotIndex();
	int32 ToIndex = this->GetSlotIndex();
	
	bool bSuccess = InventoryComponent->SwapSlots(FromIndex, ToIndex);
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("[InventorySlot] 슬롯 교환 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySlot] 슬롯 교환 실패"));
	}
	
	return bSuccess; 
}

void UOHSMInventorySlotWidget::InitializeSlot(int32 InSlotIndex, UOHSMInventoryComponent* InInventory)
{
	SlotIndex = InSlotIndex;
	InventoryComponent = InInventory;
	
	if (InventoryComponent)
	{
		const FInventorySlot* InventorySlot = InventoryComponent->GetSlot(SlotIndex);
		if (InventorySlot)
		{
			UpdateSlot(*InventorySlot);
		}
	}
}

void UOHSMInventorySlotWidget::UpdateSlot(const FInventorySlot& NewSlotData)
{
	SlotData = NewSlotData;
	RefreshUI();
}

void UOHSMInventorySlotWidget::ClearSlot()
{
	SlotData.SlotClear();
	RefreshUI();
}

const FItemData* UOHSMInventorySlotWidget::GetItemData() const
{
	if (InventoryComponent)
	{
		return InventoryComponent->GetItemData(SlotData.ItemID);
	}
	
	return nullptr;
}

void UOHSMInventorySlotWidget::RefreshUI()
{
	if (SlotData.IsEmpty())
	{
		if (ItemIcon)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		
		if (ItemCount)
		{
			ItemCount->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		const FItemData* ItemData = GetItemData();
		if (ItemData)
		{
			if (ItemIcon && ItemData->ItemIcon)
			{
				ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
				ItemIcon->SetVisibility(ESlateVisibility::Visible);
			}
			
			if (ItemCount)
			{
				if (SlotData.Count > 1)
				{
					ItemCount->SetText(FText::AsNumber(SlotData.Count));
					ItemCount->SetVisibility(ESlateVisibility::Visible);
				}
				else
				{
					ItemCount->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
	}
}
