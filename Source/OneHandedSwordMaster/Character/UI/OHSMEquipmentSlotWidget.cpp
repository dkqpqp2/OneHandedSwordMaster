// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMEquipmentSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "OneHandedSwordMaster/Character/Components/OHSMEquipmentComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Inventory/OHSMInventorySlotWidget.h"

void UOHSMEquipmentSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	UpdateUI();  // 디자이너 미리보기에서도 기본 이미지 표시
}

void UOHSMEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateUI();
}

void UOHSMEquipmentSlotWidget::InitializeSlot(EEquipmentSlot InSlotType,
	UOHSMEquipmentComponent* InEquipmentComp,
	UOHSMInventoryComponent* InInventoryComp)
{
	SlotType           = InSlotType;
	EquipmentComponent = InEquipmentComp;
	InventoryComponent = InInventoryComp;

	UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] InitializeSlot - SlotType:%d  EquipComp:%s  InvComp:%s"),
		(int32)SlotType,
		EquipmentComponent ? TEXT("OK") : TEXT("NULL"),
		InventoryComponent ? TEXT("OK") : TEXT("NULL"));

	if (EquipmentComponent)
	{
		EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &UOHSMEquipmentSlotWidget::RefreshSlot);
	}

	UpdateUI();
}

void UOHSMEquipmentSlotWidget::RefreshSlot(EEquipmentSlot ChangedSlot)
{
	UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] RefreshSlot 호출 - ChangedSlot:%d  MySlot:%d  Match:%s"),
		(int32)ChangedSlot, (int32)SlotType,
		ChangedSlot == SlotType ? TEXT("YES → UpdateUI") : TEXT("NO → skip"));

	if (ChangedSlot == SlotType)
	{
		UpdateUI();
	}
}

FReply UOHSMEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InEvent);

	if (InEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		// 우클릭: 장착 해제 → 인벤토리로 반환
		if (EquipmentComponent && !EquipmentComponent->IsSlotEmpty(SlotType))
		{
			EquipmentComponent->UnequipItem(SlotType);
		}
		return FReply::Handled();
	}

	if (InEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (EquipmentComponent && !EquipmentComponent->IsSlotEmpty(SlotType))
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}

	return FReply::Handled();
}

void UOHSMEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!EquipmentComponent || EquipmentComponent->IsSlotEmpty(SlotType))
	{
		return;
	}

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	if (DragOp)
	{
		DragOp->Payload = this;

		// 드래그 비주얼: 자기 자신 복사 위젯
		UOHSMEquipmentSlotWidget* DragVisual = CreateWidget<UOHSMEquipmentSlotWidget>(GetOwningPlayer(), GetClass());
		if (DragVisual)
		{
			DragVisual->InitializeSlot(SlotType, EquipmentComponent, InventoryComponent);
			DragVisual->SetRenderOpacity(0.7f);
			DragOp->DefaultDragVisual = DragVisual;
		}

		DragOp->Pivot   = EDragPivot::MouseDown;
		OutOperation     = DragOp;

		// 드래그 중 반투명 처리
		SetRenderOpacity(0.3f);
	}
}

bool UOHSMEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation || !EquipmentComponent || !InventoryComponent)
	{
		return false;
	}

	// 인벤토리 슬롯에서 드래그한 경우
	UOHSMInventorySlotWidget* InvSlot = Cast<UOHSMInventorySlotWidget>(InOperation->Payload);
	if (InvSlot && !InvSlot->IsEmpty())
	{
		// 슬롯 데이터에서 ItemID 가져오기
		const FInventorySlot* SlotData = InventoryComponent->GetSlot(InvSlot->GetSlotIndex());
		if (!SlotData || SlotData->IsEmpty())
		{
			return false;
		}

		// 아이템 데이터 확인 → 슬롯 타입 매칭 확인
		const FItemData* ItemData = InventoryComponent->GetItemData(SlotData->ItemID);
		if (!ItemData || ItemData->EquipSlot != SlotType)
		{
			// 슬롯 타입 불일치 → 거부
			return false;
		}

		return EquipmentComponent->EquipItem(SlotData->ItemID);
	}

	return false;
}

void UOHSMEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	SetRenderOpacity(1.0f);
}

void UOHSMEquipmentSlotWidget::UpdateUI()
{
	SetRenderOpacity(1.0f);

	UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] UpdateUI - SlotType:%d  ItemIcon:%s  EquipComp:%s"),
		(int32)SlotType,
		ItemIcon ? TEXT("OK") : TEXT("NULL"),
		EquipmentComponent ? TEXT("OK") : TEXT("NULL"));

	if (!ItemIcon)
	{
		return;
	}

	// 브러시 완전 초기화용 빈 브러시
	auto ClearIcon = [this]()
	{
		if (DefaultSlotIcon)
		{
			ItemIcon->SetBrushFromTexture(DefaultSlotIcon);
			ItemIcon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.4f));
		}
		else
		{
			// 흰 배경이 남지 않도록 브러시 자체를 투명하게 초기화
			FSlateBrush EmptyBrush;
			EmptyBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
			ItemIcon->SetBrush(EmptyBrush);
		}
	};

	if (EquipmentComponent && !EquipmentComponent->IsSlotEmpty(SlotType))
	{
		FName EquippedID = EquipmentComponent->GetEquippedItem(SlotType);
		const FItemData* Data = InventoryComponent ? InventoryComponent->GetItemData(EquippedID) : nullptr;

		UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] 장착됨 - ItemID:%s  Data:%s  Icon:%s"),
			*EquippedID.ToString(),
			Data ? TEXT("OK") : TEXT("NULL"),
			(Data && Data->ItemIcon) ? TEXT("OK") : TEXT("NULL"));

		if (Data && Data->ItemIcon)
		{
			// 장착된 아이템 아이콘 표시
			ItemIcon->SetBrushFromTexture(Data->ItemIcon);
			ItemIcon->SetColorAndOpacity(FLinearColor::White);
			ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ClearIcon();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] SlotType:%d 비어있음 → ClearIcon"), (int32)SlotType);
		ClearIcon();
	}
}

