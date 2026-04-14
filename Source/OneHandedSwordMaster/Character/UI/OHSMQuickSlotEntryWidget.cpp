// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMQuickSlotEntryWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"
#include "OneHandedSwordMaster/Character/Inventory/OHSMInventorySlotWidget.h"
#include "OneHandedSwordMaster/Character/UI/Skill/OHSMSkillDragOperation.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OneHandedSwordMaster/Data/OHSMSkillData.h"

void UOHSMQuickSlotEntryWidget::InitializeEntry(
	int32 InSlotIndex,
	const FString& InKeyLabel,
	UOHSMQuickSlotComponent* InQuickSlotComp,
	UOHSMInventoryComponent* InInventoryComp,
	UOHSMSkillComponent* InSkillComp)
{
	SlotIndex          = InSlotIndex;
	QuickSlotComponent = InQuickSlotComp;
	InventoryComponent = InInventoryComp;
	SkillComponent     = InSkillComp;

	if (TextKeyLabel)
	{
		TextKeyLabel->SetText(FText::FromString(InKeyLabel));
	}

	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
	}

	if (Img_CooldownOverlay)
	{
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
	}

	RefreshSlot();
}

bool UOHSMQuickSlotEntryWidget::IsSkillSlot() const
{
	return SlotIndex >= UOHSMQuickSlotComponent::PotionSlotCount;
}

void UOHSMQuickSlotEntryWidget::RefreshSlot()
{
	if (!QuickSlotComponent)
	{
		return;
	}

	const FName ID = QuickSlotComponent->GetSlotItemID(SlotIndex);

	// ── 스킬 슬롯 (F1~F4) ──────────────────────────────────────
	if (IsSkillSlot())
	{
		if (ID.IsNone())
		{
			if (ItemIcon)  { ItemIcon->SetVisibility(ESlateVisibility::Hidden); }
			if (TextCount) { TextCount->SetVisibility(ESlateVisibility::Hidden); }
			return;
		}

		// 스킬 컴포넌트에서 아이콘 조회
		const FOHSMSkillData* SkillData = SkillComponent ? SkillComponent->GetSkillData(ID) : nullptr;
		if (ItemIcon)
		{
			if (SkillData && SkillData->SkillIcon)
			{
				ItemIcon->SetBrushFromTexture(SkillData->SkillIcon);
				ItemIcon->SetVisibility(ESlateVisibility::Visible);
			}
		}
		if (TextCount) { TextCount->SetVisibility(ESlateVisibility::Hidden); }
		return;
	}

	// ── 포션 슬롯 (1~4) ────────────────────────────────────────
	if (!InventoryComponent)
	{
		return;
	}

	if (ID.IsNone())
	{
		if (ItemIcon)  { ItemIcon->SetVisibility(ESlateVisibility::Hidden); }
		if (TextCount) { TextCount->SetVisibility(ESlateVisibility::Hidden); }
		return;
	}

	const int32 Count = InventoryComponent->GetItemCount(ID);

	// 인벤토리에 더 이상 없으면 자동 해제
	if (Count <= 0)
	{
		QuickSlotComponent->ClearSlot(SlotIndex);
		if (ItemIcon)  { ItemIcon->SetVisibility(ESlateVisibility::Hidden); }
		if (TextCount) { TextCount->SetVisibility(ESlateVisibility::Hidden); }
		return;
	}

	const FItemData* ItemData = InventoryComponent->GetItemData(ID);
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

	// ── 스킬 패널 → 스킬 퀵슬롯 (F1~F4) 등록 ──────────────
	UOHSMSkillDragOperation* SkillDragOp = Cast<UOHSMSkillDragOperation>(InOperation);
	if (SkillDragOp && !SkillDragOp->SkillID.IsNone())
	{
		// 스킬 슬롯(4~7)에만 등록 가능
		if (!IsSkillSlot())
		{
			return false;
		}

		// 쿨타임 중에는 교체 불가
		if (IsOnCooldown())
		{
			return false;
		}

		CancelCooldown();
		QuickSlotComponent->AssignItem(SlotIndex, SkillDragOp->SkillID);
		RefreshSlot();
		return true;
	}

	// ── QuickSlot → QuickSlot 이동 ──────────────────────────
	UOHSMQuickSlotEntryWidget* DraggedQuickSlot = Cast<UOHSMQuickSlotEntryWidget>(InOperation->Payload);
	if (DraggedQuickSlot && DraggedQuickSlot != this)
	{
		DraggedQuickSlot->SetRenderOpacity(1.0f);

		// 양쪽 슬롯 중 하나라도 쿨타임 중이면 교체 불가
		if (DraggedQuickSlot->IsOnCooldown() || IsOnCooldown())
		{
			return false;
		}

		DraggedQuickSlot->CancelCooldown();
		CancelCooldown();
		QuickSlotComponent->SwapSlots(DraggedQuickSlot->GetSlotIndex(), SlotIndex);
		DraggedQuickSlot->RefreshSlot();
		RefreshSlot();
		return true;
	}

	// ── Inventory → 포션 퀵슬롯 등록 ────────────────────────
	// 스킬 슬롯에는 인벤토리 아이템 드롭 불가
	if (IsSkillSlot())
	{
		return false;
	}

	UOHSMInventorySlotWidget* DraggedSlot = Cast<UOHSMInventorySlotWidget>(InOperation->Payload);
	if (!DraggedSlot || DraggedSlot->IsEmpty())
	{
		return false;
	}

	if (!InventoryComponent)
	{
		return false;
	}

	const FInventorySlot* InvSlot = InventoryComponent->GetSlot(DraggedSlot->GetSlotIndex());
	if (!InvSlot || InvSlot->IsEmpty())
	{
		return false;
	}

	// 포션 슬롯(0~3)에는 소비 아이템만 등록 가능
	const FItemData* ItemData = InventoryComponent->GetItemData(InvSlot->ItemID);
	if (!ItemData || ItemData->ItemType != EItemType::Consumable)
	{
		return false;
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

	// 드래그 비주얼: 자기 자신 복사본 생성 (원본은 제자리 유지)
	UOHSMQuickSlotEntryWidget* DragVisual = CreateWidget<UOHSMQuickSlotEntryWidget>(GetOwningPlayer(), GetClass());
	if (DragVisual)
	{
		DragVisual->InitializeEntry(SlotIndex, TEXT(""), QuickSlotComponent, InventoryComponent, SkillComponent);
		DragVisual->SetRenderOpacity(0.7f);
		DragOp->DefaultDragVisual = DragVisual;
	}

	DragOp->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOp;

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

void UOHSMQuickSlotEntryWidget::CancelCooldown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	CooldownTotal   = 0.f;
	CooldownElapsed = 0.f;

	if (IsValid(CooldownMID))
	{
		CooldownMID->SetScalarParameterValue(TEXT("Progress"), 0.f);
	}
	if (Img_CooldownOverlay)
	{
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
	if (TextCooldown)
	{
		TextCooldown->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UOHSMQuickSlotEntryWidget::StartCooldown(float CooldownDuration)
{
	if (CooldownDuration <= 0.f || !Img_CooldownOverlay)
	{
		return;
	}

	// MID 최초 1회 생성
	if (!IsValid(CooldownMID) && CooldownMaterial)
	{
		CooldownMID = UMaterialInstanceDynamic::Create(CooldownMaterial, this);
		Img_CooldownOverlay->SetBrushFromMaterial(CooldownMID);
	}

	if (!IsValid(CooldownMID))
	{
		return;
	}

	CooldownTotal   = CooldownDuration;
	CooldownElapsed = 0.f;

	CooldownMID->SetScalarParameterValue(TEXT("Progress"), 1.f);
	Img_CooldownOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 쿨다운 텍스트 표시
	if (TextCooldown)
	{
		TextCooldown->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), CooldownDuration)));
		TextCooldown->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// 기존 타이머 정리 후 0.05s 간격으로 재시작
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		World->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&UOHSMQuickSlotEntryWidget::OnCooldownTick,
			0.05f,
			true);
	}
}

void UOHSMQuickSlotEntryWidget::OnCooldownTick()
{
	CooldownElapsed += 0.05f;

	if (CooldownElapsed >= CooldownTotal)
	{
		// 쿨다운 완료
		if (IsValid(CooldownMID))
		{
			CooldownMID->SetScalarParameterValue(TEXT("Progress"), 0.f);
		}
		if (Img_CooldownOverlay)
		{
			Img_CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
		if (TextCooldown)
		{
			TextCooldown->SetVisibility(ESlateVisibility::Hidden);
		}
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		}
		return;
	}

	// Progress: 1(시작) → 0(완료)
	const float Progress = 1.f - (CooldownElapsed / CooldownTotal);
	if (IsValid(CooldownMID))
	{
		CooldownMID->SetScalarParameterValue(TEXT("Progress"), Progress);
	}

	// 남은 시간 텍스트 갱신
	if (TextCooldown)
	{
		const float Remaining = CooldownTotal - CooldownElapsed;
		TextCooldown->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Remaining)));
	}
}
