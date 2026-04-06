// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMEquipmentWidget.h"

#include "OHSMEquipmentSlotWidget.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMEquipmentComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

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
	UE_LOG(LogTemp, Warning, TEXT("[EquipWidget] RefreshStats 호출됨 - ChangedSlot:%d"), (int32)ChangedSlot);
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
	// SelfHitTestInvisible: 위젯 자신은 이벤트 통과, 자식 슬롯들은 정상 클릭 가능
	// → Equipment 배경이 Inventory 이벤트를 막지 않음
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UOHSMEquipmentWidget::CloseEquipment()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UOHSMEquipmentWidget::ToggleEquipment()
{
	// IsVisible()로 체크 (SelfHitTestInvisible도 visible로 인식)
	if (IsVisible())
	{
		CloseEquipment();
	}
	else
	{
		OpenEquipment();
	}
}
