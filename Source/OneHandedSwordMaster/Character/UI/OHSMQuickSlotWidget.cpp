// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMQuickSlotWidget.h"

#include "OHSMQuickSlotEntryWidget.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"

const TArray<FString> UOHSMQuickSlotWidget::PotionKeyLabels = { TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4") };
const TArray<FString> UOHSMQuickSlotWidget::SkillKeyLabels  = { TEXT("F1"), TEXT("F2"), TEXT("F3"), TEXT("F4") };

void UOHSMQuickSlotWidget::InitializeSlots(
	UOHSMQuickSlotComponent* InQuickSlotComp,
	UOHSMInventoryComponent* InInventoryComp,
	UOHSMSkillComponent*     InSkillComp)
{
	// ── 이전 구독 해제 ─────────────────────────────────────────────────────────
	// 맵 전환이나 재초기화 시 기존 컴포넌트의 델리게이트에서 이 위젯을 제거.
	// AddUniqueDynamic 과 함께 사용해 누적 바인딩을 완전히 차단한다.
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnInventoryUpdated.RemoveAll(this);
	}
	if (IsValid(SkillComponent))
	{
		SkillComponent->OnSkillActivated.RemoveAll(this);
	}

	QuickSlotComponent = InQuickSlotComp;
	SkillComponent     = InSkillComp;

	const int32 Offset = GetBaseSlotOffset();
	const TArray<FString>& KeyLabels = (SlotGroupType == EQuickSlotGroupType::Potion)
		? PotionKeyLabels
		: SkillKeyLabels;

	TArray<UOHSMQuickSlotEntryWidget*> Entries = GetAllEntries();
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i])
		{
			Entries[i]->InitializeEntry(Offset + i, KeyLabels[i], InQuickSlotComp, InInventoryComp, InSkillComp);
		}
	}

	// 포션 슬롯만 인벤토리 변경 시 갱신 (스킬 슬롯은 인벤토리와 무관)
	if (InInventoryComp && SlotGroupType == EQuickSlotGroupType::Potion)
	{
		InventoryComponent = InInventoryComp; // 해제용 캐시 갱신
		// AddUniqueDynamic: 동일 (오브젝트, 함수) 쌍이 이미 있으면 추가하지 않음
		InInventoryComp->OnInventoryUpdated.AddUniqueDynamic(this, &UOHSMQuickSlotWidget::OnInventoryChanged);
	}

	// 스킬 슬롯이면 OnSkillActivated 델리게이트 구독 → 쿨다운 전달
	if (InSkillComp && SlotGroupType == EQuickSlotGroupType::Skill)
	{
		InSkillComp->OnSkillActivated.AddUObject(this, &UOHSMQuickSlotWidget::OnSkillActivated);
	}
}

void UOHSMQuickSlotWidget::NativeDestruct()
{
	// 인벤토리·스킬 델리게이트 구독 모두 해제 (위젯 소멸 시 스테일 참조 방지)
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnInventoryUpdated.RemoveAll(this);
	}

	if (IsValid(SkillComponent))
	{
		SkillComponent->OnSkillActivated.RemoveAll(this);
	}

	Super::NativeDestruct();
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

void UOHSMQuickSlotWidget::OnSkillActivated(FName SkillID, float Cooldown)
{
	if (!QuickSlotComponent || SkillID.IsNone() || Cooldown <= 0.f)
	{
		return;
	}
	
	for (UOHSMQuickSlotEntryWidget* Entry : GetAllEntries())
	{
		if (!Entry)
		{
			continue;
		}

		const FName SlotItemID = QuickSlotComponent->GetSlotItemID(Entry->GetSlotIndex());
		if (SlotItemID == SkillID)
		{
			Entry->StartCooldown(Cooldown);
		}
	}
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
