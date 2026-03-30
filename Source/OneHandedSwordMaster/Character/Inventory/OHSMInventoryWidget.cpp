// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMInventoryWidget.h"

#include "OHSMInventorySlotWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Engine/UserInterfaceSettings.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

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
                CanvasSlot->SetSize(CurrentSize);
                CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
            }
        }
    }
}


void UOHSMInventoryWidget::NativeDestruct()
{
	if (InventoryComponent && IsValid(InventoryComponent))
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UOHSMInventoryWidget::OnInventorySlotUpdated);
		InventoryComponent->OnInventorySizeChanged.RemoveDynamic(this, &UOHSMInventoryWidget::OnInventorySizeChanged);
	}
	
	Super::NativeDestruct();
}

void UOHSMInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UOHSMInventoryWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::I)
	{
		CloseInventory();
		return FReply::Handled();
	}
	
	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UOHSMInventoryWidget::InitializeInventory(UOHSMInventoryComponent* InInventory)
{
	
	if (!InInventory)
	{
		return;
	}
	
	InventoryComponent = InInventory;
	
	InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UOHSMInventoryWidget::OnInventorySlotUpdated);
	InventoryComponent->OnInventorySizeChanged.AddDynamic(this, &UOHSMInventoryWidget::OnInventorySizeChanged);
	
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
	
	if (AOHSMPlayerController* PlayerController = Cast<AOHSMPlayerController>(GetOwningPlayer()))
	{
		FInputModeGameOnly GameOnlyMode;
		
		PlayerController->SetInputMode(GameOnlyMode);
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetIsInventoryOpen(false);
	}
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
		EResizeHandle Handle = GetResizeHandleAtPosition(InGeometry, InMouseEvent);
		if (Handle != EResizeHandle::None)
		{
			bIsResizing = true;
			CurrentResizeHandle = Handle;
			ResizeStartMousePos = InMouseEvent.GetScreenSpacePosition();

			if (MainBorder && MainBorder->Slot)
			{
				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MainBorder->Slot))
				{
					ResizeStartSize = CanvasSlot->GetSize();
					ResizeStartPosition = CanvasSlot->GetPosition();
				}
			}
			// 리사이즈도 마우스 캡처 필요
			return FReply::Handled().CaptureMouse(TakeWidget());
		}

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
		bool bWasDragging = bIsDragging;
		bool bWasResizing = bIsResizing;
		
		bIsDragging = false;
		bIsResizing = false;
		CurrentResizeHandle = EResizeHandle::None;
		
		if (bWasDragging || bWasResizing)
		{
			return FReply::Handled().ReleaseMouseCapture();
		}
	}
    
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UOHSMInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (bIsDragging || bIsResizing)
		{
			bIsDragging = false;
			bIsResizing = false;
			CurrentResizeHandle = EResizeHandle::None;
            
			return FReply::Handled().ReleaseMouseCapture();
		}
        
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}
	
	float DPIScale = GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(FIntPoint(GEngine->GameViewport->Viewport->GetSizeXY()));
	
	if (bIsResizing)
	{
		FVector2D CurrentMousePos = InMouseEvent.GetScreenSpacePosition();
		FVector2D MouseDelta = (CurrentMousePos - ResizeStartMousePos) / DPIScale;
		
		FVector2D NewSize = ResizeStartSize;
		FVector2D NewPosition = ResizeStartPosition;
		
		switch (CurrentResizeHandle)
		{
			case EResizeHandle::Right:
				NewSize.X = ResizeStartSize.X + MouseDelta.X;
				break;
			case EResizeHandle::Bottom:
				NewSize.Y = ResizeStartSize.Y + MouseDelta.Y;
				break;
			case EResizeHandle::BottomRight:
				NewSize.X = ResizeStartSize.X + MouseDelta.X;
				NewSize.Y = ResizeStartSize.Y + MouseDelta.Y;
				break;
			case EResizeHandle::BottomLeft:
				NewSize.X = ResizeStartSize.X - MouseDelta.X;
				NewSize.Y = ResizeStartSize.Y + MouseDelta.Y;
				NewPosition.X = ResizeStartPosition.X + MouseDelta.X;
				break;
			default:
				break;
		}
		NewSize.X = FMath::Clamp(NewSize.X, MinSize.X, MaxSize.X);
		NewSize.Y = FMath::Clamp(NewSize.Y, MinSize.Y, MaxSize.Y);
		
		SetWidgetSize(NewSize);
		
		if (CurrentResizeHandle == EResizeHandle::BottomLeft)
		{
			SetWidgetPosition(NewPosition);
		}
		
		UpdateGridColumns();
		return FReply::Handled();
	}
	
	if (bIsDragging)
	{
		FVector2D CurrentMousePos = InMouseEvent.GetScreenSpacePosition();
		FVector2D MouseDelta = CurrentMousePos - DragStartMousePos;
		
		MouseDelta = MouseDelta / DPIScale;
		
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

EResizeHandle UOHSMInventoryWidget::GetResizeHandleAtPosition(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	const float HandleSize = 10.0f;  // 테두리 감지 영역 크기
    
    if (!MainBorder)
    {
        return EResizeHandle::None;
    }
	
    FGeometry MainBorderGeometry = MainBorder->GetCachedGeometry();
    FVector2D LocalMousePos = MainBorderGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    FVector2D BorderSize = MainBorderGeometry.GetLocalSize();

    // 우하단 핸들
    if (ResizeHandleBottomRight)
    {
        FGeometry HandleGeo = ResizeHandleBottomRight->GetCachedGeometry();
        FVector2D HandleLocalPos = HandleGeo.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        FVector2D HandleSize2D = HandleGeo.GetLocalSize();
        
        if (HandleLocalPos.X >= 0 && HandleLocalPos.X <= HandleSize2D.X &&
            HandleLocalPos.Y >= 0 && HandleLocalPos.Y <= HandleSize2D.Y)
        {
            return EResizeHandle::BottomRight;
        }
    }
    
    // 우측 핸들
    if (ResizeHandleRight)
    {
        FGeometry HandleGeo = ResizeHandleRight->GetCachedGeometry();
        FVector2D HandleLocalPos = HandleGeo.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        FVector2D HandleSize2D = HandleGeo.GetLocalSize();
        
        if (HandleLocalPos.X >= 0 && HandleLocalPos.X <= HandleSize2D.X &&
            HandleLocalPos.Y >= 0 && HandleLocalPos.Y <= HandleSize2D.Y)
        {
            return EResizeHandle::Right;
        }
    }
    
    // 하단 핸들
    if (ResizeHandleBottom)
    {
        FGeometry HandleGeo = ResizeHandleBottom->GetCachedGeometry();
        FVector2D HandleLocalPos = HandleGeo.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        FVector2D HandleSize2D = HandleGeo.GetLocalSize();
        
        if (HandleLocalPos.X >= 0 && HandleLocalPos.X <= HandleSize2D.X &&
            HandleLocalPos.Y >= 0 && HandleLocalPos.Y <= HandleSize2D.Y)
        {
            return EResizeHandle::Bottom;
        }
    }
    
    // 좌하단 핸들
    if (ResizeHandleBottomLeft)
    {
        FGeometry HandleGeo = ResizeHandleBottomLeft->GetCachedGeometry();
        FVector2D HandleLocalPos = HandleGeo.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        FVector2D HandleSize2D = HandleGeo.GetLocalSize();
        
        if (HandleLocalPos.X >= 0 && HandleLocalPos.X <= HandleSize2D.X &&
            HandleLocalPos.Y >= 0 && HandleLocalPos.Y <= HandleSize2D.Y)
        {
            return EResizeHandle::BottomLeft;
        }
    }

    bool bNearRight = LocalMousePos.X >= BorderSize.X - HandleSize;
    bool bNearBottom = LocalMousePos.Y >= BorderSize.Y - HandleSize;
    bool bNearLeft = LocalMousePos.X <= HandleSize;
	
    if (bNearRight && bNearBottom)
    {
	    return EResizeHandle::BottomRight;
    }
    
    if (bNearLeft && bNearBottom)
    {
	    return EResizeHandle::BottomLeft;
    }
	
    if (bNearRight)
    {
	    return EResizeHandle::Right;
    }
    
    if (bNearBottom)
    {
	    return EResizeHandle::Bottom;
    }
    
    return EResizeHandle::None;
}

void UOHSMInventoryWidget::SetWidgetSize(FVector2D NewSize)
{
	CurrentSize = NewSize;
    
	if (MainBorder && MainBorder->Slot)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MainBorder->Slot))
		{
			CanvasSlot->SetSize(NewSize);
		}
	}
}

void UOHSMInventoryWidget::UpdateGridColumns()
{
	const float SlotWidth = 70.0f;
	const float SlotPadding = 8.0f;
	const float ContentPadding = 20.0f;
	const float TotalSlotWidth = SlotWidth + SlotPadding;
    
	float AvailableWidth = CurrentSize.X - ContentPadding;
	int32 NewColumns = FMath::Floor(AvailableWidth / TotalSlotWidth);
	
	NewColumns = FMath::Clamp(NewColumns, 3, 20);
    
	if (NewColumns != GridColumns)
	{
		GridColumns = NewColumns;
		
		if (InventoryComponent)
		{
			CreateSlots(InventoryComponent->GetSlotCount());
		}
	}
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
