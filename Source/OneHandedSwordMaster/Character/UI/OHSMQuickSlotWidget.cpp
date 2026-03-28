// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMQuickSlotWidget.h"

#include "OHSMQuickSlotEntryWidget.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"

const TArray<FString> UOHSMQuickSlotWidget::PotionKeyLabels = { TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4") };
const TArray<FString> UOHSMQuickSlotWidget::SkillKeyLabels  = { TEXT("F1"), TEXT("F2"), TEXT("F3"), TEXT("F4") };

void UOHSMQuickSlotWidget::InitializeSlots(
	UOHSMQuickSlotComponent* InQuickSlotComp,
	UOHSMInventoryComponent* InInventoryComp)
{
	const int32 Offset = GetBaseSlotOffset();
	const TArray<FString>& KeyLabels = (SlotGroupType == EQuickSlotGroupType::Potion)
		? PotionKeyLabels
		: SkillKeyLabels;

	TArray<UOHSMQuickSlotEntryWidget*> Entries = GetAllEntries();
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i])
		{
			Entries[i]->InitializeEntry(Offset + i, KeyLabels[i], InQuickSlotComp, InInventoryComp);
		}
	}

	// 인벤토리가 바뀔 때마다 개수 갱신
	if (InInventoryComp)
	{
		InInventoryComp->OnInventoryUpdated.AddDynamic(this, &UOHSMQuickSlotWidget::OnInventoryChanged);
	}
}

void UOHSMQuickSlotWidget::RefreshAllSlots()
{
	for (UOHSMQuickSlotEntryWidget* Entry : GetAllEntries())
	{
		if (Entry)
		{
			Entry->RefreshSlot();
		}
	}
}

void UOHSMQuickSlotWidget::OnInventoryChanged(int32 ChangedSlotIndex)
{
	RefreshAllSlots();
}

int32 UOHSMQuickSlotWidget::GetBaseSlotOffset() const
{
	return (SlotGroupType == EQuickSlotGroupType::Potion)
		? 0
		: UOHSMQuickSlotComponent::PotionSlotCount;
}

TArray<UOHSMQuickSlotEntryWidget*> UOHSMQuickSlotWidget::GetAllEntries() const
{
	return { QuickSlotEntry_0, QuickSlotEntry_1, QuickSlotEntry_2, QuickSlotEntry_3 };
}
