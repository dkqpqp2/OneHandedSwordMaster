// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMInventoryWidget.h"

#include "OHSMInventorySlotWidget.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"

void UOHSMInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UOHSMInventoryWidget::OnCloseButtonClicked);
	}
	
	ApplySize();
}

void UOHSMInventoryWidget::NativeDestruct()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UOHSMInventoryWidget::OnInventorySlotUpdated);
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UOHSMInventoryWidget::OnInventorySizeChanged);
	}
	
	Super::NativeDestruct();
}

void UOHSMInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UOHSMInventoryWidget::InitializeInventory(UOHSMInventoryComponent* InInventory)
{
	
	if (!InInventory)
	{
		return;
	}
	
	InventoryComponent = InInventory;
	
	InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UOHSMInventoryWidget::OnInventorySlotUpdated);
	InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UOHSMInventoryWidget::OnInventorySizeChanged);
	
	int32 SlotCount = InventoryComponent->GetSlotCount();
	CreateSlots(SlotCount);
	
}

void UOHSMInventoryWidget::CreateSlots(int32 SlotCount)
{
	if (!SlotGrid || !SlotWidgetClass)
	{
		return;
	}
	
	SlotGrid->ClearChildren();
	SlotWidgets.Empty();
	
	for (int32 i = 0; i < SlotCount; i++)
	{
		UOHSMInventorySlotWidget* SlotWidget = CreateWidget<UOHSMInventorySlotWidget>(this, SlotWidgetClass);
		if (SlotWidget)
		{
			SlotWidget->InitializeSlot(i, InventoryComponent);
			
			SlotWidgets.Add(SlotWidget);
			
			int32 Row = i / GridColumns;
			int32 Column = i % GridColumns;
			
			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(SlotWidget, Row, Column);
			
		}
	}
}

void UOHSMInventoryWidget::UpdateSlot(int32 SlotIndex)
{
	if (!InventoryComponent || !SlotWidgets.IsValidIndex(SlotIndex))
	{
		return;
	}
    
	const FInventorySlot* SlotData = InventoryComponent->GetSlot(SlotIndex);
    
	if (SlotData && SlotWidgets[SlotIndex])
	{
		SlotWidgets[SlotIndex]->UpdateSlot(*SlotData);
	}
}

void UOHSMInventoryWidget::RefreshAllSlots()
{
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		UpdateSlot(i);
	}
}

void UOHSMInventoryWidget::OpenInventory()
{
	SetVisibility(ESlateVisibility::Visible);
	RefreshAllSlots();
}

void UOHSMInventoryWidget::CloseInventory()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UOHSMInventoryWidget::ToggleInventory()
{
	if (GetVisibility() == ESlateVisibility::Visible)
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}

void UOHSMInventoryWidget::SetInventorySize(FVector2D NewSize)
{
	CurrentSize.X = FMath::Clamp(NewSize.X, MinSize.X, MaxSize.X);
	CurrentSize.Y = FMath::Clamp(NewSize.Y, MinSize.Y, MaxSize.Y);
	
	ApplySize();
}

void UOHSMInventoryWidget::OnInventorySlotUpdated(int32 SlotIndex)
{
	UpdateSlot(SlotIndex);
}

void UOHSMInventoryWidget::OnInventorySizeChanged(int32 NewSlotCount)
{
	CreateSlots(NewSlotCount);
}

void UOHSMInventoryWidget::OnCloseButtonClicked()
{
	CloseInventory();
}

FReply UOHSMInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UOHSMInventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UOHSMInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UOHSMInventoryWidget::SetWidgetPosition(FVector2D NewPosition)
{
}

void UOHSMInventoryWidget::ApplySize()
{
	if (InventorySizeBox)
	{
		InventorySizeBox->SetWidthOverride(CurrentSize.X);
		InventorySizeBox->SetHeightOverride(CurrentSize.Y);
	}
}
