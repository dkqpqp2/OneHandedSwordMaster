// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMSkillNodeWidget.h"

#include "OHSMSkillDragOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOHSMSkillNodeWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOHSMSkillNodeWidget::InitNode(FName InSkillID, const FOHSMSkillData* InData)
{
	SkillID   = InSkillID;
	SkillData = InData;

	if (!InData)
	{
		return;
	}

	// 아이콘 설정
	if (Img_Icon && InData->SkillIcon)
	{
		Img_Icon->SetBrushFromTexture(InData->SkillIcon);
	}

	// 이름 설정
	if (Label_Name)
	{
		Label_Name->SetText(InData->SkillName);
	}

	// Btn_Node 는 비주얼 전용 — 이벤트는 UserWidget 레벨에서 처리
	// (Button 이 HitTest 를 가로채지 않도록 HitTestInvisible 로 설정)
	if (Btn_Node)
	{
		Btn_Node->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	RefreshState(ESkillNodeState::Locked);
}

void UOHSMSkillNodeWidget::RefreshState(ESkillNodeState NewState)
{
	CurrentState = NewState;

	if (!Img_StateFrame || !Img_LockOverlay) return;

	switch (NewState)
	{
	case ESkillNodeState::Locked:
		Img_StateFrame->SetColorAndOpacity(LockedColor);
		Img_LockOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		break;

	case ESkillNodeState::Available:
		Img_StateFrame->SetColorAndOpacity(AvailableColor);
		Img_LockOverlay->SetVisibility(ESlateVisibility::Collapsed);
		break;

	case ESkillNodeState::Learned:
		Img_StateFrame->SetColorAndOpacity(LearnedColor);
		Img_LockOverlay->SetVisibility(ESlateVisibility::Collapsed);
		break;
	}
}

void UOHSMSkillNodeWidget::OnBtnNodeClicked()
{
	OnNodeClicked.Broadcast(SkillID);
}

// ─── 드래그 & 클릭 ──────────────────────────────────────────────────────────

FReply UOHSMSkillNodeWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		bIsDragging = false;
		// 드래그 임계값 감지 시작 (이동 없이 릴리즈하면 MouseButtonUp 으로 클릭 처리)
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UOHSMSkillNodeWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// 배운 스킬이고 패시브가 아닌 경우에만 퀵슬롯으로 드래그 가능
	if (CurrentState != ESkillNodeState::Learned || !SkillData || SkillData->SkillType == ESkillType::Passive)
	{
		// 드래그 불가 — OutOperation = nullptr 이면 드래그 시작 안 됨
		return;
	}

	bIsDragging = true;

	UOHSMSkillDragOperation* DragOp = NewObject<UOHSMSkillDragOperation>(this);
	DragOp->SkillID   = SkillID;
	DragOp->SkillIcon = SkillData->SkillIcon;
	DragOp->Payload   = this;

	// 드래그 비주얼: 자신과 동일한 노드 위젯 복사본
	UOHSMSkillNodeWidget* DragVisual = CreateWidget<UOHSMSkillNodeWidget>(GetOwningPlayer(), GetClass());
	if (DragVisual)
	{
		DragVisual->InitNode(SkillID, SkillData);
		DragVisual->RefreshState(ESkillNodeState::Learned);
		DragVisual->SetRenderOpacity(0.7f);
		DragOp->DefaultDragVisual = DragVisual;
	}

	DragOp->Pivot  = EDragPivot::MouseDown;
	OutOperation   = DragOp;

	SetRenderOpacity(0.5f);
}

void UOHSMSkillNodeWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	bIsDragging = false;
	SetRenderOpacity(1.0f);
}

FReply UOHSMSkillNodeWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	// 드래그 없이 마우스를 뗐을 때만 클릭으로 처리 (스킬 배우기 팝업)
	if (!bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnNodeClicked.Broadcast(SkillID);
	}

	return FReply::Handled();
}
