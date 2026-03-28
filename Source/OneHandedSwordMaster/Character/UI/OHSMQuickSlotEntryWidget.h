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
	void InitializeEntry(
		int32 InSlotIndex,
		const FString& InKeyLabel,
		class UOHSMQuickSlotComponent* InQuickSlotComp,
		class UOHSMInventoryComponent* InInventoryComp);

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

public:
	int32 GetSlotIndex() const { return SlotIndex; }
	class UOHSMQuickSlotComponent* GetQuickSlotComponent() const { return QuickSlotComponent; }

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
};
