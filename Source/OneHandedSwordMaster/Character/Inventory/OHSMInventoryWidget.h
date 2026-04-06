// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMInventoryWidget.generated.h"


/**
 * 
 */
UENUM(BlueprintType)
enum class EResizeHandle : uint8
{
	None,
	TopLeft,
	Top,
	TopRight,
	Right,
	BottomRight,
	Bottom,
	BottomLeft,
	Left
};

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> MainBorder;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidget> TitleBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TitleText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UUniformGridPanel> SlotGrid;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScrollBox> ScrollContainer;
	
	// ResizeHandle
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> ResizeHandleBottomRight;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> ResizeHandleRight;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> ResizeHandleBottom;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> ResizeHandleBottomLeft;
	
protected:
	UPROPERTY()
	TArray<TObjectPtr<class UOHSMInventorySlotWidget>> SlotWidgets;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Slot")
	TSubclassOf<class UOHSMInventorySlotWidget> SlotWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "1", ClampMax = "20"))
	int32 GridColumns = 5;
	
	UPROPERTY()
	TObjectPtr<class UOHSMInventoryComponent> InventoryComponent;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Size")
	FVector2D MinSize = FVector2D(440, 400);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Size")
	FVector2D MaxSize = FVector2D(1200, 900);
	
	UPROPERTY()
	FVector2D CurrentSize = FVector2D(520, 480);
	
protected:
	UPROPERTY()
	bool bIsDragging = false;
	
	UPROPERTY()
	FVector2D DragStartMousePos;
	
	UPROPERTY()
	FVector2D DragStartWidgetPos;
	
	// Resize 상태
	UPROPERTY()
	bool bIsResizing = false;
    
	UPROPERTY()
	EResizeHandle CurrentResizeHandle = EResizeHandle::None;
    
	UPROPERTY()
	FVector2D ResizeStartMousePos;
    
	UPROPERTY()
	FVector2D ResizeStartSize;
    
	UPROPERTY()
	FVector2D ResizeStartPosition;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(UOHSMInventoryComponent* InInventory);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateSlots(int32 SlotCount);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateSlot(int32 SlotIndex);
	
    UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshAllSlots();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenInventory();
    
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory();
    
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();
	
protected:
	UFUNCTION()
	void OnInventorySlotUpdated(int32 SlotIndex);
	
	UFUNCTION()
	void OnInventorySizeChanged(int32 NewSlotCount);
	
	UFUNCTION()
	void OnCloseButtonClicked();
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	bool IsMouseOverTitleBar(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	
	EResizeHandle GetResizeHandleAtPosition(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	
	void SetWidgetPosition(FVector2D NewPosition);
	
	void SetWidgetSize(FVector2D NewSize);
    
	void UpdateGridColumns();
};
