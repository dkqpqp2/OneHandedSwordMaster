// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMExpBar.h"

#include "Components/ProgressBar.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMCharacterInterface.h"


void UOHSMExpBar::NativeConstruct()
{
	Super::NativeConstruct();
	
	ExpProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PbExpBar")));
	
	IOHSMCharacterInterface* CharacterWidget = Cast<IOHSMCharacterInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}
}

void UOHSMExpBar::UpdateExpBar(int32 CurrentExp, int32 RequiredExp)
{
	if (ExpProgressBar)
	{
		float Percent = RequiredExp > 0 ? static_cast<float>(CurrentExp) / static_cast<float>(RequiredExp) : 0.0f;
		ExpProgressBar->SetPercent(Percent);
	}
}
