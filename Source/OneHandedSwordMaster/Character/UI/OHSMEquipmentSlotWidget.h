// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OneHandedSwordMaster/Character/Inventory/OHSMInventoryTooltipWidget.h"
#include "OHSMEquipmentSlotWidget.generated.h"

/**
 * 장비창 개별 슬롯 위젯
 * - 인벤토리에서 드래그 드롭으로 장착
 * - 우클릭으로 해제 (인벤토리로 반환)
 * - 장착된 아이템 아이콘 표시
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

protected:
	/** 장착된 아이템 아이콘 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemIcon;

	/** 마우스 오버 하이라이트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> HoverHighlight;

protected:
	/** 장비가 없을 때 표시할 기본 이미지 (Blueprint에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
	TObjectPtr<class UTexture2D> DefaultSlotIcon;

	/** 툴팁 위젯 클래스 (Blueprint에서 WBP_InventoryTooltip 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot|Tooltip")
	TSubclassOf<UOHSMInventoryTooltipWidget> TooltipWidgetClass;

	UPROPERTY()
	TObjectPtr<UOHSMInventoryTooltipWidget> SlotTooltipWidget;

protected:
	/** 이 슬롯의 타입 (무기/투구/갑옷/바지/신발) */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment Slot")
	EEquipmentSlot SlotType = EEquipmentSlot::None;

	UPROPERTY()
	TObjectPtr<class UOHSMEquipmentComponent> EquipmentComponent;

	UPROPERTY()
	TObjectPtr<class UOHSMInventoryComponent> InventoryComponent;

public:
	void InitializeSlot(EEquipmentSlot InSlotType,
		class UOHSMEquipmentComponent* InEquipmentComp,
		class UOHSMInventoryComponent* InInventoryComp);
	
	UFUNCTION()
	void RefreshSlot(EEquipmentSlot ChangedSlot);

	EEquipmentSlot GetSlotType() const { return SlotType; }
	class UOHSMEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

private:
	void UpdateUI();
};
