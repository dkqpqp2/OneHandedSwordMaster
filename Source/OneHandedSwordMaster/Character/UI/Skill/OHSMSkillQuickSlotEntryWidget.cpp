// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMSkillQuickSlotEntryWidget.h"

#include "OHSMSkillDragOperation.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"
#include "OneHandedSwordMaster/Data/OHSMSkillData.h"

// ─── 초기화 ─────────────────────────────────────────────────────────────────

void UOHSMSkillQuickSlotEntryWidget::InitializeEntry(
	int32 InSkillSlotIndex,
	const FString& InKeyLabel,
	UOHSMQuickSlotComponent* InQuickSlotComp,
	UOHSMSkillComponent* InSkillComp)
{
	SkillSlotIndex     = InSkillSlotIndex;
	QuickSlotComponent = InQuickSlotComp;
	SkillComponent     = InSkillComp;

	if (Text_KeyLabel)
	{
		Text_KeyLabel->SetText(FText::FromString(InKeyLabel));
	}

	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
	}

	// 쿨다운 머티리얼 인스턴스 생성
	if (CooldownMaterial && Img_CooldownOverlay)
	{
		CooldownMID = UMaterialInstanceDynamic::Create(CooldownMaterial, this);
		Img_CooldownOverlay->SetBrushFromMaterial(CooldownMID);
		CooldownMID->SetScalarParameterValue(TEXT("Progress"), 0.f);
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
	}

	RefreshSlot();
}

// ─── 슬롯 갱신 ──────────────────────────────────────────────────────────────

void UOHSMSkillQuickSlotEntryWidget::RefreshSlot()
{
	if (!QuickSlotComponent || !SkillComponent)
	{
		return;
	}

	const FName SkillID = GetAssignedSkillID();

	if (SkillID.IsNone())
	{
		if (Img_SkillIcon)
		{
			Img_SkillIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	const FOHSMSkillData* SkillData = SkillComponent->GetSkillData(SkillID);
	if (!SkillData)
	{
		if (Img_SkillIcon)
		{
			Img_SkillIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	if (Img_SkillIcon && SkillData->SkillIcon)
	{
		Img_SkillIcon->SetBrushFromTexture(SkillData->SkillIcon);
		Img_SkillIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

FName UOHSMSkillQuickSlotEntryWidget::GetAssignedSkillID() const
{
	if (!QuickSlotComponent)
	{
		return NAME_None;
	}
	return QuickSlotComponent->GetSlotItemID(GetComponentSlotIndex());
}

int32 UOHSMSkillQuickSlotEntryWidget::GetComponentSlotIndex() const
{
	return UOHSMQuickSlotComponent::PotionSlotCount + SkillSlotIndex;
}

// ─── 쿨다운 ─────────────────────────────────────────────────────────────────

void UOHSMSkillQuickSlotEntryWidget::StartCooldown(float CooldownDuration)
{
	if (CooldownDuration <= 0.f || !CooldownMID)
	{
		return;
	}

	CooldownTotal   = CooldownDuration;
	CooldownElapsed = 0.f;

	// 오버레이 표시 + Progress = 1 (전체 회색)
	CooldownMID->SetScalarParameterValue(TEXT("Progress"), 1.f);

	if (Img_CooldownOverlay)
	{
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// 기존 타이머 초기화 후 새로 시작
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&UOHSMSkillQuickSlotEntryWidget::OnCooldownTick,
		0.05f,
		true
	);
}

void UOHSMSkillQuickSlotEntryWidget::OnCooldownTick()
{
	CooldownElapsed += 0.05f;

	// Progress: 1(쿨다운 직후) → 0(쿨다운 완료), 시계 방향으로 회색이 사라짐
	const float Progress = 1.f - FMath::Clamp(CooldownElapsed / CooldownTotal, 0.f, 1.f);

	if (CooldownMID)
	{
		CooldownMID->SetScalarParameterValue(TEXT("Progress"), Progress);
	}

	if (CooldownElapsed >= CooldownTotal)
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);

		if (Img_CooldownOverlay)
		{
			Img_CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// ─── 드래그 & 드롭 ──────────────────────────────────────────────────────────

bool UOHSMSkillQuickSlotEntryWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation || !QuickSlotComponent)
	{
		return false;
	}

	// ── 스킬 패널 → 스킬 퀵슬롯 등록 ─────────────────────────────
	UOHSMSkillDragOperation* SkillDragOp = Cast<UOHSMSkillDragOperation>(InOperation);
	if (SkillDragOp && !SkillDragOp->SkillID.IsNone())
	{
		QuickSlotComponent->AssignItem(GetComponentSlotIndex(), SkillDragOp->SkillID);
		RefreshSlot();
		return true;
	}

	// ── 스킬 퀵슬롯 → 스킬 퀵슬롯 스왑 ─────────────────────────
	UOHSMSkillQuickSlotEntryWidget* DraggedEntry = Cast<UOHSMSkillQuickSlotEntryWidget>(InOperation->Payload);
	if (DraggedEntry && DraggedEntry != this)
	{
		DraggedEntry->SetRenderOpacity(1.f);
		QuickSlotComponent->SwapSlots(DraggedEntry->GetComponentSlotIndex(), GetComponentSlotIndex());
		DraggedEntry->RefreshSlot();
		RefreshSlot();
		return true;
	}

	return false;
}

FReply UOHSMSkillQuickSlotEntryWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 좌클릭: 슬롯에 스킬이 있으면 드래그 시작
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		const bool bHasSkill = QuickSlotComponent && !GetAssignedSkillID().IsNone();
		if (bHasSkill)
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}

	// 우클릭: 슬롯 해제
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (QuickSlotComponent)
		{
			QuickSlotComponent->ClearSlot(GetComponentSlotIndex());
			RefreshSlot();
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UOHSMSkillQuickSlotEntryWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>(this);
	DragOp->Payload = this;

	// 드래그 비주얼: 자신과 동일한 위젯 복사본
	UOHSMSkillQuickSlotEntryWidget* DragVisual =
		CreateWidget<UOHSMSkillQuickSlotEntryWidget>(GetOwningPlayer(), GetClass());
	if (DragVisual)
	{
		DragVisual->InitializeEntry(SkillSlotIndex, TEXT(""), QuickSlotComponent, SkillComponent);
		DragVisual->SetRenderOpacity(0.7f);
		DragOp->DefaultDragVisual = DragVisual;
	}

	DragOp->Pivot  = EDragPivot::MouseDown;
	OutOperation   = DragOp;

	SetRenderOpacity(0.5f);
}

void UOHSMSkillQuickSlotEntryWidget::NativeOnDragCancelled(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	SetRenderOpacity(1.f);
}

void UOHSMSkillQuickSlotEntryWidget::NativeOnMouseEnter(
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

void UOHSMSkillQuickSlotEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
	}
}
