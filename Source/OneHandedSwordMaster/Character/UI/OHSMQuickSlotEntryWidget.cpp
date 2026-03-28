// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMQuickSlotEntryWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "OneHandedSwordMaster/Character/Inventory/OHSMInventorySlotWidget.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"

void UOHSMQuickSlotEntryWidget::InitializeEntry(
	int32 InSlotIndex,
	const FString& InKeyLabel,
	UOHSMQuickSlotComponent* InQuickSlotComp,
	UOHSMInventoryComponent* InInventoryComp)
{
	SlotIndex         = InSlotIndex;
	QuickSlotComponent = InQuickSlotComp;
	InventoryComponent = InInventoryComp;

	if (TextKeyLabel)
	{
		TextKeyLabel->SetText(FText::FromString(InKeyLabel));
	}

	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
	}

	RefreshSlot();
}

void UOHSMQuickSlotEntryWidget::RefreshSlot()
{
	if (!QuickSlotComponent || !InventoryComponent)
	{
		return;
	}

	FName ItemID = QuickSlotComponent->GetSlotItemID(SlotIndex);

	if (ItemID.IsNone())
	{
		if (ItemIcon)  ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (TextCount) TextCount->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	int32 Count = InventoryComponent->GetItemCount(ItemID);

	// 인벤토리에 더 이상 없으면 자동 해제
	if (Count <= 0)
	{
		QuickSlotComponent->ClearSlot(SlotIndex);
		if (ItemIcon)  ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (TextCount) TextCount->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const FItemData* ItemData = InventoryComponent->GetItemData(ItemID);
	if (!ItemData)
	{
		return;
	}

	if (ItemIcon && ItemData->ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}

	if (TextCount)
	{
		TextCount->SetText(FText::AsNumber(Count));
		TextCount->SetVisibility(ESlateVisibility::Visible);
	}
}

bool UOHSMQuickSlotEntryWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation || !QuickSlotComponent || !InventoryComponent)
	{
		return false;
	}

	UOHSMInventorySlotWidget* DraggedSlot = Cast<UOHSMInventorySlotWidget>(InOperation->Payload);
	if (!DraggedSlot || DraggedSlot->IsEmpty())
	{
		return false;
	}

	const FInventorySlot* InvSlot = InventoryComponent->GetSlot(DraggedSlot->GetSlotIndex());
	if (!InvSlot || InvSlot->IsEmpty())
	{
		return false;
	}

	// 포션 슬롯(0~3)에는 소비 아이템만 등록 가능
	if (SlotIndex < UOHSMQuickSlotComponent::PotionSlotCount)
	{
		const FItemData* ItemData = InventoryComponent->GetItemData(InvSlot->ItemID);
		if (!ItemData || ItemData->ItemType != EItemType::Consumable)
		{
			return false;
		}
	}

	QuickSlotComponent->AssignItem(SlotIndex, InvSlot->ItemID);
	RefreshSlot();
	return true;
}

FReply UOHSMQuickSlotEntryWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 좌클릭: 아이템이 있으면 드래그 시작
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		bool bHasItem = QuickSlotComponent && !QuickSlotComponent->GetSlotItemID(SlotIndex).IsNone();
		if (bHasItem)
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}

	// 우클릭: 슬롯 해제
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (QuickSlotComponent)
		{
			QuickSlotComponent->ClearSlot(SlotIndex);
			RefreshSlot();
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UOHSMQuickSlotEntryWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	if (!DragOp)
	{
		return;
	}

	DragOp->Payload = this;

	// 드래그 비주얼: 현재 슬롯과 동일하게 복사
	UOHSMQuickSlotEntryWidget* DragVisual = CreateWidget<UOHSMQuickSlotEntryWidget>(GetOwningPlayer(), GetClass());
	if (DragVisual)
	{
		DragVisual->InitializeEntry(SlotIndex, TEXT(""), QuickSlotComponent, InventoryComponent);
		DragVisual->SetRenderOpacity(0.7f);
		DragOp->DefaultDragVisual = DragVisual;
	}

	DragOp->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOp;

	// 드래그 중 원본 반투명 처리
	SetRenderOpacity(0.5f);
}

void UOHSMQuickSlotEntryWidget::NativeOnDragCancelled(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	// 드래그 취소 시 원상복구
	SetRenderOpacity(1.0f);
}

void UOHSMQuickSlotEntryWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::HitTestInvisible);
		HoverHighlight->SetRenderOpacity(0.3f);
	}
}

void UOHSMQuickSlotEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
	}
}
