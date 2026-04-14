// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBuffBarWidget.h"

#include "OHSMBuffStatusWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"

// ─── 초기화 ─────────────────────────────────────────────────────────────────

void UOHSMBuffBarWidget::InitializeBar(UOHSMSkillComponent* InSkillComp)
{
	SkillComponent = InSkillComp;

	if (!InSkillComp)
	{
		return;
	}

	InSkillComp->OnBuffApplied.AddUObject(this, &UOHSMBuffBarWidget::OnBuffApplied);
	InSkillComp->OnBuffExpired.AddUObject(this, &UOHSMBuffBarWidget::OnBuffExpired);
}

void UOHSMBuffBarWidget::NativeDestruct()
{
	if (SkillComponent)
	{
		SkillComponent->OnBuffApplied.RemoveAll(this);
		SkillComponent->OnBuffExpired.RemoveAll(this);
	}

	Super::NativeDestruct();
}

// ─── 버프 추가·제거 ──────────────────────────────────────────────────────────

void UOHSMBuffBarWidget::OnBuffApplied(FName SkillID, UTexture2D* Icon, float Duration)
{
	if (!Box_Buffs || !BuffStatusWidgetClass)
	{
		return;
	}

	// 이미 같은 버프가 표시 중이면 시간만 갱신
	if (TObjectPtr<UOHSMBuffStatusWidget>* Existing = ActiveBuffWidgets.Find(SkillID))
	{
		if (IsValid(*Existing))
		{
			(*Existing)->InitBuff(SkillID, Icon, Duration);
			(*Existing)->SetVisibility(ESlateVisibility::Visible);
			return;
		}
	}

	// 새 버프 아이콘 위젯 생성
	UOHSMBuffStatusWidget* NewWidget =
		CreateWidget<UOHSMBuffStatusWidget>(GetOwningPlayer(), BuffStatusWidgetClass);

	if (!NewWidget)
	{
		return;
	}

	NewWidget->InitBuff(SkillID, Icon, Duration);

	// HorizontalBox 에 추가 (8px 간격)
	UHorizontalBoxSlot* BoxSlot = Box_Buffs->AddChildToHorizontalBox(NewWidget);
	if (BoxSlot)
	{
		FSlateChildSize SlotSize;
		SlotSize.SizeRule = ESlateSizeRule::Automatic;
		BoxSlot->SetSize(SlotSize);

		FMargin RightPadding(0.f, 0.f, 8.f, 0.f);
		BoxSlot->SetPadding(RightPadding);
	}

	ActiveBuffWidgets.Add(SkillID, NewWidget);
}

void UOHSMBuffBarWidget::OnBuffExpired(FName SkillID)
{
	if (TObjectPtr<UOHSMBuffStatusWidget>* Found = ActiveBuffWidgets.Find(SkillID))
	{
		if (IsValid(*Found))
		{
			(*Found)->ForceExpire();

			// HorizontalBox 에서 제거
			if (Box_Buffs)
			{
				Box_Buffs->RemoveChild(*Found);
			}
		}

		ActiveBuffWidgets.Remove(SkillID);
	}
}
