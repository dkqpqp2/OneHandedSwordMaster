// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPlayerController.h"

#include "OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/UI/OHSMHUDWidget.h"
#include "OneHandedSwordMaster/Character/Inventory/OHSMInventoryWidget.h"

AOHSMPlayerController::AOHSMPlayerController()
{
	static ConstructorHelpers::FClassFinder<UOHSMHUDWidget> HUDWidgetClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/OneHandedSwordMaster/UI/WBP_OHSMHUD.WBP_OHSMHUD_C'"));
	if (HUDWidgetClassRef.Class)
	{
		HUDWidgetClass = HUDWidgetClassRef.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UOHSMInventoryWidget> InventoryWidgetClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/OneHandedSwordMaster/UI/WBP_Inventory.WBP_Inventory_C'"));
	if (InventoryWidgetClassRef.Class)
	{
		InventoryWidgetClass = InventoryWidgetClassRef.Class;
	}
}

void AOHSMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	FInputModeGameOnly GameOnlyInputMode;
	
	SetInputMode(GameOnlyInputMode);
	
	InitializeHUDWidget();
	InitializeInventoryWidget();
}

void AOHSMPlayerController::InitializeHUDWidget()
{
	if (!HUDWidgetClass)
	{
		return;
	}
	
	OHSMHUDWidget = CreateWidget<UOHSMHUDWidget>(this, HUDWidgetClass);
	if (!OHSMHUDWidget)
	{
		return;
	}
	
	OHSMHUDWidget->AddToViewport(0);
}

void AOHSMPlayerController::InitializeInventoryWidget()
{
	if (!InventoryWidgetClass)
	{
		return;
	}
	
	InventoryWidget = CreateWidget<UOHSMInventoryWidget>(this, InventoryWidgetClass);
	
	if (!InventoryWidget)
	{
		return;
	}
	
	InventoryWidget->AddToPlayerScreen(10);
	
	AOHSMPlayerCharacter* PlayerCharacter = Cast<AOHSMPlayerCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		UOHSMInventoryComponent* InventoryComp = PlayerCharacter->GetInventoryComponent();
		if (InventoryComp)
		{
			InventoryWidget->InitializeInventory(InventoryComp);
		}
	}
	
	InventoryWidget->CloseInventory();
}

void AOHSMPlayerController::ToggleInventory()
{
	if (!InventoryWidget)
	{
		return;
	}
	
	if (InventoryWidget)
	{
		bool bWillOpen = (InventoryWidget->GetVisibility() != ESlateVisibility::Visible);
		
		InventoryWidget->ToggleInventory();
		
		if (bWillOpen)
		{
			FInputModeUIOnly UIOnlyMode;
			
			UIOnlyMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
			UIOnlyMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			
			SetInputMode(UIOnlyMode);
			SetShowMouseCursor(true);
			
			bIsInventoryOpen = true;
		}
		else
		{
			FInputModeGameOnly GameOnlyMode;
        
			SetInputMode(GameOnlyMode);
			SetShowMouseCursor(false);
        
			bIsInventoryOpen = false;
		}
	}
}
