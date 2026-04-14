// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMSkillQuickSlotBarWidget.h"

#include "OHSMSkillQuickSlotEntryWidget.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"

// ─── 초기화 ─────────────────────────────────────────────────────────────────

void UOHSMSkillQuickSlotBarWidget::InitializeBar(
	UOHSMQuickSlotComponent* InQuickSlotComp,
	UOHSMSkillComponent*     InSkillComp)
{
	SkillComponent = InSkillComp;

	// 단축키 라벨 (F1 ~ F4)
	const TArray<FString> KeyLabels = { TEXT("F1"), TEXT("F2"), TEXT("F3"), TEXT("F4") };

	const TArray<UOHSMSkillQuickSlotEntryWidget*> Entries = GetAllEntries();
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i])
		{
			Entries[i]->InitializeEntry(i, KeyLabels[i], InQuickSlotComp, InSkillComp);
		}
	}

	// 스킬 발동 시 쿨다운 시작 이벤트 바인딩
	if (InSkillComp)
	{
		InSkillComp->OnSkillActivated.AddUObject(this, &UOHSMSkillQuickSlotBarWidget::OnSkillActivated);
	}
}

void UOHSMSkillQuickSlotBarWidget::RefreshAllSlots()
{
	for (UOHSMSkillQuickSlotEntryWidget* Entry : GetAllEntries())
	{
		if (Entry)
		{
			Entry->RefreshSlot();
		}
	}
}

void UOHSMSkillQuickSlotBarWidget::NativeDestruct()
{
	// 델리게이트 해제
	if (SkillComponent)
	{
		SkillComponent->OnSkillActivated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

// ─── 내부 ────────────────────────────────────────────────────────────────────

TArray<UOHSMSkillQuickSlotEntryWidget*> UOHSMSkillQuickSlotBarWidget::GetAllEntries() const
{
	return { SkillSlot_0, SkillSlot_1, SkillSlot_2, SkillSlot_3 };
}

void UOHSMSkillQuickSlotBarWidget::OnSkillActivated(FName SkillID, float CooldownDuration)
{
	// 해당 SkillID 가 등록된 슬롯을 찾아 쿨다운 UI 시작
	for (UOHSMSkillQuickSlotEntryWidget* Entry : GetAllEntries())
	{
		if (Entry && Entry->GetAssignedSkillID() == SkillID)
		{
			Entry->StartCooldown(CooldownDuration);
			break;
		}
	}
}
