// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMWidgetComponent.h"
#include "OneHandedSwordMaster/Character/UI/OHSMUserWidget.h"

void UOHSMWidgetComponent::InitWidget()
{
	Super::InitWidget();
	
	UOHSMUserWidget* OHSMUserWidget = Cast<UOHSMUserWidget>(GetWidget());
	if (OHSMUserWidget)
	{
		OHSMUserWidget->SetOwningActor(GetOwner());
	}
}
