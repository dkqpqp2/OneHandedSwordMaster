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
	SlotType = InSlotType;
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

	// 툴팁 위젯 생성
	if (TooltipWidgetClass && GetOwningPlayer())
	{
		SlotTooltipWidget = CreateWidget<UOHSMInventoryTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
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
		const bool bHasItem = EquipmentComponent && !EquipmentComponent->IsSlotEmpty(SlotType);
		UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] 좌클릭 - SlotType:%d  HasItem:%s"),
			(int32)SlotType, bHasItem ? TEXT("YES") : TEXT("NO"));

		if (bHasItem)
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}

	return FReply::Handled();
}

FReply UOHSMEquipmentSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InEvent);
	return FReply::Handled().ReleaseMouseCapture();
}

void UOHSMEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!EquipmentComponent || EquipmentComponent->IsSlotEmpty(SlotType))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] DragDetected 취소 - EquipComp:%s  SlotEmpty:%s"),
			EquipmentComponent ? TEXT("OK") : TEXT("NULL"),
			(!EquipmentComponent || EquipmentComponent->IsSlotEmpty(SlotType)) ? TEXT("YES") : TEXT("NO"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] DragDetected 시작 - SlotType:%d"), (int32)SlotType);

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

		DragOp->Pivot    = EDragPivot::MouseDown;
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

	// 드롭 대상 없이 취소된 경우 → 장착 해제 (인벤토리로 자동 반환)
	if (EquipmentComponent && !EquipmentComponent->IsSlotEmpty(SlotType))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EquipSlot] DragCancelled → UnequipItem SlotType:%d"), (int32)SlotType);
		EquipmentComponent->UnequipItem(SlotType);
	}

	SetRenderOpacity(1.0f);
}

void UOHSMEquipmentSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::HitTestInvisible);
		HoverHighlight->SetRenderOpacity(0.3f);
	}
}

void UOHSMEquipmentSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
		HoverHighlight->SetRenderOpacity(0.0f);
	}
}

// TintColor를 포함한 Brush를 명시적으로 흰색으로 설정하는 헬퍼
static void SetIconTexture(UImage* Icon, UTexture2D* Texture)
{
	if (!Icon || !Texture) return;

	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.TintColor = FSlateColor(FLinearColor::White);  // TintColor 명시적으로 흰색
	Icon->SetBrush(Brush);
	Icon->SetColorAndOpacity(FLinearColor::White);       // 위젯 컬러도 흰색
}

void UOHSMEquipmentSlotWidget::UpdateUI()
{
	SetRenderOpacity(1.0f);

	// HoverHighlight 항상 초기화 (Blueprint 기본값 덮어쓰기)
	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
		HoverHighlight->SetRenderOpacity(0.0f);
	}

	if (!ItemIcon)
	{
		return;
	}

	// 장비 없을 때 슬롯 아이콘 초기화
	auto ClearIcon = [this]()
	{
		if (DefaultSlotIcon)
		{
			SetIconTexture(ItemIcon, DefaultSlotIcon);
			ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			// DefaultSlotIcon 없으면 완전히 투명하게 (흰 박스 방지)
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		// 장비 없으면 툴팁 제거
		SetToolTip(nullptr);
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
			// 장착된 아이템 아이콘 표시 (100% 불투명)
			SetIconTexture(ItemIcon, Data->ItemIcon);
			ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);

			// 툴팁 갱신
			if (SlotTooltipWidget)
			{
				SlotTooltipWidget->UpdateTooltip(*Data);
				SetToolTip(SlotTooltipWidget);
			}
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

