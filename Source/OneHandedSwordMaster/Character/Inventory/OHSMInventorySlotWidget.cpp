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
		
		if (GetClass())
		{
			UOHSMInventorySlotWidget* DragVisual = CreateWidget<UOHSMInventorySlotWidget>(GetWorld(), GetClass());
            
			if (DragVisual)
			{
				DragVisual->UpdateSlot(SlotData);
				DragVisual->SetRenderOpacity(0.7f);
                
				DragDropOp->DefaultDragVisual = DragVisual;
			}
		}
        
		DragDropOp->Pivot = EDragPivot::MouseDown;
        
		OutOperation = DragDropOp;
        
		SetDragging(true);
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
	
	DraggedSlot->SetDragging(false);

	int32 FromIndex = DraggedSlot->GetSlotIndex();
	int32 ToIndex = this->GetSlotIndex();
	
	if (FromIndex == ToIndex)
	{
		return false;
	}
	
	bool bSuccess = InventoryComponent->SwapSlots(FromIndex, ToIndex);
	
	return bSuccess; 
}

void UOHSMInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::HitTestInvisible);
		HoverHighlight->SetRenderOpacity(0.3f);
	}
}

void UOHSMInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
		HoverHighlight->SetRenderOpacity(0.0f);
	}
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

void UOHSMInventorySlotWidget::SetDragging(bool bIsDragging)
{
	if (bIsDragging)
	{
		SetRenderOpacity(0.5f);
	}
	else
	{
		SetRenderOpacity(1.0f);
	}
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
