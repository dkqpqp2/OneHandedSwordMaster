// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPlayerController.h"
#include "OneHandedSwordMaster/Character/UI/OHSMHUDWidget.h"

AOHSMPlayerController::AOHSMPlayerController()
{
	static ConstructorHelpers::FClassFinder<UOHSMHUDWidget> HUDWidgetClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/OneHandedSwordMaster/UI/WBP_OHSMHUD.WBP_OHSMHUD_C'"));
	if (HUDWidgetClassRef.Class)
	{
		HUDWidgetClass = HUDWidgetClassRef.Class;
	}
}

void AOHSMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	FInputModeGameOnly GameOnlyInputMode;
	
	SetInputMode(GameOnlyInputMode);
	
	OHSMHUDWidget = CreateWidget<UOHSMHUDWidget>(this, HUDWidgetClass);
	if (OHSMHUDWidget)
	{
		OHSMHUDWidget->AddToViewport();
	}
}
