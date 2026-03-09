// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMHpBar.h"

#include "Components/ProgressBar.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMCharacterInterface.h"

UOHSMHpBar::UOHSMHpBar(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxHp = -1.0f;
}

void UOHSMHpBar::NativeConstruct()
{ 
	Super::NativeConstruct();
	
	HpProgressBar = Cast<UProgressBar>(GetWidgetFromName("PbHpBar"));

	IOHSMCharacterInterface* CharacterWidget = Cast<IOHSMCharacterInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}
}

void UOHSMHpBar::UpdateHpBar(float CurrentHp, float InMaxHp)
{
	MaxHp = InMaxHp;
    
	// HP 바 업데이트
	if (HpProgressBar)
	{
		float Percent = MaxHp > 0.0f ? CurrentHp / MaxHp : 0.0f;
		HpProgressBar->SetPercent(Percent);
	}
}
