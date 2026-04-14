// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMSkillData.h"
#include "OHSMSkillNodeWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillNodeClicked, FName /*SkillID*/);

/** 스킬 노드 상태 */
UENUM(BlueprintType)
enum class ESkillNodeState : uint8
{
	Locked,		// 전제 조건 미충족 or 포인트 부족
	Available,	// 지금 바로 배울 수 있음
	Learned,	// 이미 배운 스킬
};

/**
 * 스킬 트리의 노드 하나.
 * Canvas_SkillTree 위에 TreeTier / TreeColumn 좌표로 배치된다.
 *
 * [Blueprint BindWidget]
 *   Btn_Node        - 클릭 가능한 버튼 (루트)
 *   Img_Icon        - 스킬 아이콘
 *   Img_StateFrame  - 노드 테두리 (상태별 색상 변경)
 *   Img_LockOverlay - 잠금 상태일 때만 보이는 어두운 오버레이
 *   Label_Name      - 스킬 이름 (짧게)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMSkillNodeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 노드 초기화 — 스킬 패널에서 호출 */
	void InitNode(FName InSkillID, const FOHSMSkillData* InData);

	/** 습득 상태 갱신 (스킬 배우거나 포인트 변동 시) */
	void RefreshState(ESkillNodeState NewState);

	ESkillNodeState GetNodeState() const { return CurrentState; }
	FName GetSkillID() const { return SkillID; }

	/** 노드 클릭 시 스킬 패널로 알림 */
	FOnSkillNodeClicked OnNodeClicked;

protected:
	virtual void NativePreConstruct() override;

	// ─── BindWidget ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Node;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_StateFrame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_LockOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_Name;

	// ─── 상태별 테두리 색상 ───────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Node")
	FLinearColor LockedColor    = FLinearColor(0.2f, 0.2f, 0.2f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Node")
	FLinearColor AvailableColor = FLinearColor(0.9f, 0.75f, 0.1f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Node")
	FLinearColor LearnedColor   = FLinearColor(0.1f, 0.8f, 0.3f, 1.f);

protected:
	// ─── 드래그 & 클릭 ───────────────────────────────────────────
	/** 좌클릭(+드래그 감지) */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 드래그 시작 — Learned + 비패시브이면 UOHSMSkillDragOperation 생성 */
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	/** 드래그 취소 — 원본 투명도 복구 */
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** 마우스 업 — 드래그가 아닌 단순 클릭이면 OnNodeClicked 발행 */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	FName SkillID;
	const FOHSMSkillData* SkillData = nullptr;
	ESkillNodeState CurrentState = ESkillNodeState::Locked;

	/** 드래그 중인지 여부 (MouseUp에서 클릭 발행 방지) */
	bool bIsDragging = false;

	UFUNCTION()
	void OnBtnNodeClicked();
};
