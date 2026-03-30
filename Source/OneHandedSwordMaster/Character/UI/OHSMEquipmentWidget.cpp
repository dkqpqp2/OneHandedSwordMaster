// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMEquipmentWidget.h"

#include "OHSMEquipmentSlotWidget.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMEquipmentComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"

void UOHSMEquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Hidden);
}

void UOHSMEquipmentWidget::InitializeEquipment(UOHSMEquipmentComponent* EquipComp,
	UOHSMInventoryComponent* InvComp,
	UOHSMPlayerStatComponent* InStatComp)
{
	StatComponent = InStatComp;

	auto Init = [&](UOHSMEquipmentSlotWidget* EquipSlot, EEquipmentSlot Type)
	{
		if (EquipSlot)
		{
			EquipSlot->InitializeSlot(Type, EquipComp, InvComp);
		}
	};

	Init(EquipmentSlot_Weapon, EEquipmentSlot::Weapon);
	Init(EquipmentSlot_Helmet, EEquipmentSlot::Helmet);
	Init(EquipmentSlot_Armor,  EEquipmentSlot::Armor);
	Init(EquipmentSlot_Pants,  EEquipmentSlot::Pants);
	Init(EquipmentSlot_Shoes,  EEquipmentSlot::Shoes);

	// 장비 변경 시 스탯 갱신
	if (EquipComp)
	{
		EquipComp->OnEquipmentChanged.AddDynamic(this, &UOHSMEquipmentWidget::RefreshStats);
	}

	UpdateStatsDisplay();
}

void UOHSMEquipmentWidget::RefreshStats(EEquipmentSlot ChangedSlot)
{
	UpdateStatsDisplay();
}

void UOHSMEquipmentWidget::UpdateStatsDisplay()
{
	if (!StatComponent)
	{
		return;
	}

	if (TextAtk)
	{
		TextAtk->SetText(FText::FromString(
			FString::Printf(TEXT("공격력 : %.0f"), StatComponent->GetAttackPower())));
	}
	if (TextDef)
	{
		TextDef->SetText(FText::FromString(
			FString::Printf(TEXT("방어력 : %.0f"), StatComponent->GetDefensePower())));
	}
	if (TextHp)
	{
		TextHp->SetText(FText::FromString(
			FString::Printf(TEXT("체력 : %.0f"), StatComponent->GetMaxHp())));
	}
	if (TextMp)
	{
		TextMp->SetText(FText::FromString(
			FString::Printf(TEXT("마나 : %.0f"), StatComponent->GetMaxMana())));
	}
}

void UOHSMEquipmentWidget::OpenEquipment()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UOHSMEquipmentWidget::CloseEquipment()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UOHSMEquipmentWidget::ToggleEquipment()
{
	if (GetVisibility() == ESlateVisibility::Visible)
	{
		CloseEquipment();
	}
	else
	{
		OpenEquipment();
	}
}
