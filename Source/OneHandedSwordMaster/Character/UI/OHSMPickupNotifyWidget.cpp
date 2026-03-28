// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMPickupNotifyWidget.h"

#include "OHSMPickupNotifyEntryWidget.h"
#include "Components/VerticalBox.h"

void UOHSMPickupNotifyWidget::ShowItemPickup(const FItemData& ItemData, int32 Count)
{
	// 같은 아이템이 이미 표시 중이면 맨 아래로 이동 후 개수 갱신
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		UOHSMPickupNotifyEntryWidget* Entry = Entries[i];
		if (Entry && Entry->GetItemID() == ItemData.ItemID)
		{
			Entry->AddCount(Count);

			// 맨 아래로 이동 (VerticalBox에서 꺼냈다가 다시 추가)
			Entry->RemoveFromParent();
			NotifyList->AddChild(Entry);

			// Entries 배열도 맨 뒤로 이동
			Entries.RemoveAt(i);
			Entries.Add(Entry);
			return;
		}
	}

	// 새 항목 생성
	if (!EntryWidgetClass || !NotifyList)
	{
		return;
	}

	UOHSMPickupNotifyEntryWidget* NewEntry = CreateWidget<UOHSMPickupNotifyEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
	if (!NewEntry)
	{
		return;
	}

	NewEntry->InitializeEntry(ItemData, Count);
	NotifyList->AddChild(NewEntry);
	Entries.Add(NewEntry);

	// 최대 개수 초과 시 가장 오래된 항목 제거
	if (Entries.Num() > MaxEntries)
	{
		if (Entries[0])
		{
			Entries[0]->RemoveFromParent();
		}
		Entries.RemoveAt(0);
	}
}

void UOHSMPickupNotifyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 만료된 항목 뒤에서부터 제거 (앞에서 제거하면 인덱스 꼬임)
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (!Entries[i] || Entries[i]->IsExpired())
		{
			if (Entries[i])
			{
				Entries[i]->RemoveFromParent();
			}
			Entries.RemoveAt(i);
		}
	}
}
