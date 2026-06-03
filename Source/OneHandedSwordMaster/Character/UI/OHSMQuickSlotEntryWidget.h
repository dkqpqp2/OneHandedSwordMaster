// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMQuickSlotEntryWidget.generated.h"

/**
 * 퀵슬롯 단일 슬롯 위젯
 * - 인벤토리 슬롯을 드래그해서 올려놓으면 아이템 등록
 * - 우클릭으로 슬롯 해제
 * - 인벤토리가 변경될 때 자동으로 개수 갱신
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMQuickSlotEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯 초기화 (스킬 슬롯이면 InSkillComp 전달)
	void InitializeEntry(
		int32 InSlotIndex,
		const FString& InKeyLabel,
		class UOHSMQuickSlotComponent* InQuickSlotComp,
		class UOHSMInventoryComponent* InInventoryComp,
		class UOHSMSkillComponent* InSkillComp = nullptr);

	// 슬롯 표시 갱신 (인벤토리 변경 시 외부에서 호출)
	void RefreshSlot();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextCount;

	// 단축키 라벨 (1/2/3/4 또는 Q/E/R/F)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextKeyLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> HoverHighlight;

	// 쿨다운 원형 오버레이 (스킬 슬롯 전용)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> Img_CooldownOverlay;

	// 쿨다운 남은 시간 텍스트
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextCooldown;

	// 쿨다운 머티리얼 (BP 지정)
	UPROPERTY(EditAnywhere, Category = "Cooldown")
	TObjectPtr<class UMaterialInterface> CooldownMaterial;

public:
	int32 GetSlotIndex() const { return SlotIndex; }
	class UOHSMQuickSlotComponent* GetQuickSlotComponent() const { return QuickSlotComponent; }

	// 쿨다운 오버레이 시작
	void StartCooldown(float CooldownDuration);

	// 쿨다운 즉시 초기화
	void CancelCooldown();

	// 쿨다운 중 여부
	bool IsOnCooldown() const { return CooldownTotal > 0.f && CooldownElapsed < CooldownTotal; }

protected:
	// 인벤토리 슬롯 드랍 수신
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// 좌클릭: 드래그 시작 / 우클릭: 슬롯 해제
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 드래그 감지 시 드래그 오퍼레이션 생성
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// 드래그 취소 시 슬롯 원상복구
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	int32 SlotIndex = -1;

	UPROPERTY()
	TObjectPtr<class UOHSMQuickSlotComponent> QuickSlotComponent;

	UPROPERTY()
	TObjectPtr<class UOHSMInventoryComponent> InventoryComponent;

	// 스킬 컴포넌트 참조 (스킬 슬롯용)
	UPROPERTY()
	TObjectPtr<class UOHSMSkillComponent> SkillComponent;

	// 스킬 슬롯 여부 확인
	bool IsSkillSlot() const;

	// ─── 쿨다운 ─────────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> CooldownMID;

	FTimerHandle CooldownTimerHandle;
	float CooldownTotal   = 0.f;
	float CooldownElapsed = 0.f;

	UFUNCTION()
	void OnCooldownTick();
};
