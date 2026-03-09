// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMManaBar.h"

#include "Components/ProgressBar.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMCharacterInterface.h"

UOHSMManaBar::UOHSMManaBar(const FObjectInitializer& OHSMManaBarInitializer) : Super(OHSMManaBarInitializer)
{
	MaxMana = -1.0f;
}

void UOHSMManaBar::NativeConstruct()
{
	Super::NativeConstruct();
	
	ManaProgressBar = Cast<UProgressBar>(GetWidgetFromName("PbManaBar"));
	
	IOHSMCharacterInterface* CharacterWidget = Cast<IOHSMCharacterInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}
	
}

void UOHSMManaBar::UpdateManaBar(float CurrentMana, float InMaxMana)
{
	MaxMana = InMaxMana;
	
	if (ManaProgressBar)
	{
		float Percent = MaxMana > 0.0f ? CurrentMana / MaxMana : 0.0f;
		ManaProgressBar->SetPercent(Percent);
	}
}
