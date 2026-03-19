// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMInventoryWidget.h"

#include "OHSMInventorySlotWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/UniformGridPanel.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"

void UOHSMInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
	
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UOHSMInventoryWidget::OnCloseButtonClicked);
    }
	
    if (MainBorder)
    {
        UPanelSlot* BorderSlot = MainBorder->Slot;
        
        if (BorderSlot)
        {
            UE_LOG(LogTemp, Display, TEXT("[InventoryWidget] MainBorder Slot 타입: %s"),
                *BorderSlot->GetClass()->GetName());
        	
            if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(BorderSlot))
            {
                CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
                CanvasSlot->SetPosition(FVector2D(100, 100));
                CanvasSlot->SetSize(FVector2D(520, 480));
                CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
                
                FVector2D Pos = CanvasSlot->GetPosition();
                FVector2D Size = CanvasSlot->GetSize();
            }
        }
    }
}


void UOHSMInventoryWidget::NativeDestruct()
{
	if (InventoryComponent && IsValid(InventoryComponent))
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UOHSMInventoryWidget::OnInventorySlotUpdated);
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UOHSMInventoryWidget::OnInventorySizeChanged);
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
			
			SlotGrid->AddChildToUniformGrid(SlotWidget, Row, Column);
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
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
        
		if (IsMouseOverTitleBar(InGeometry, InMouseEvent))
		{
            
			bIsDragging = true;
			DragStartMousePos = InMouseEvent.GetScreenSpacePosition();
			
			if (MainBorder && MainBorder->Slot)
			{
				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MainBorder->Slot))
				{
					DragStartWidgetPos = CanvasSlot->GetPosition();
				}
			}
			return FReply::Handled().CaptureMouse(TakeWidget());
		}
	}
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UOHSMInventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsDragging)
		{
			bIsDragging = false;
            
			return FReply::Handled().ReleaseMouseCapture();
		}
	}
    
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UOHSMInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging)
	{
		FVector2D CurrentMousePos = InMouseEvent.GetScreenSpacePosition();
		FVector2D MouseDelta = CurrentMousePos - DragStartMousePos;
		FVector2D NewPosition = DragStartWidgetPos + MouseDelta;
        
		SetWidgetPosition(NewPosition);
        
		return FReply::Handled();
	}
    
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

bool UOHSMInventoryWidget::IsMouseOverTitleBar(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!TitleBar)
	{
		return false;
	}
    
	FGeometry TitleBarGeometry = TitleBar->GetCachedGeometry();
	FVector2D LocalMousePos = TitleBarGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D TitleBarSize = TitleBarGeometry.GetLocalSize();
    
	bool bIsOver = LocalMousePos.X >= 0 &&
				   LocalMousePos.X <= TitleBarSize.X &&
				   LocalMousePos.Y >= 0 &&
				   LocalMousePos.Y <= TitleBarSize.Y;
    
	return bIsOver;
}

void UOHSMInventoryWidget::SetWidgetPosition(FVector2D NewPosition)
{
	if (MainBorder && MainBorder->Slot)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MainBorder->Slot))
		{
			CanvasSlot->SetPosition(NewPosition);
		}
	}
}
