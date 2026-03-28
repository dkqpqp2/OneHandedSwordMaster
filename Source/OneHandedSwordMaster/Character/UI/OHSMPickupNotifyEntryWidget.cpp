// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMPickupNotifyEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOHSMPickupNotifyEntryWidget::InitializeEntry(const FItemData& InItemData, int32 Count)
{
	ItemID = InItemData.ItemID;
	TotalCount = Count;
	ElapsedTime = 0.0f;
	bExpired = false;
	SetRenderOpacity(1.0f);

	if (ItemIcon && InItemData.ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(InItemData.ItemIcon);
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(InItemData.ItemName);
	}

	if (ItemCountText)
	{
		ItemCountText->SetText(FText::Format(
			NSLOCTEXT("Pickup", "ItemCount", "x{0}"),
			FText::AsNumber(TotalCount)));
	}
}

void UOHSMPickupNotifyEntryWidget::AddCount(int32 AdditionalCount)
{
	TotalCount += AdditionalCount;

	// 타이머 리셋 → 다시 처음부터 카운트
	ElapsedTime = 0.0f;
	bExpired = false;
	SetRenderOpacity(1.0f);

	if (ItemCountText)
	{
		ItemCountText->SetText(FText::Format(
			NSLOCTEXT("Pickup", "ItemCount", "x{0}"),
			FText::AsNumber(TotalCount)));
	}
}

void UOHSMPickupNotifyEntryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bExpired)
	{
		return;
	}

	ElapsedTime += InDeltaTime;

	// DisplayTime + FadeOutTime 이 지나면 완전히 만료
	if (ElapsedTime >= DisplayTime + FadeOutTime)
	{
		bExpired = true;
		SetRenderOpacity(0.0f);
		return;
	}

	// DisplayTime 이 지나면 위로 올라가면서 페이드아웃
	if (ElapsedTime >= DisplayTime)
	{
		// 0.0 ~ 1.0 (0: 페이드 시작, 1: 완전히 사라짐)
		float FadeProgress = (ElapsedTime - DisplayTime) / FadeOutTime;

		// 투명도 감소
		SetRenderOpacity(1.0f - FadeProgress);

		// 위로 이동 (RenderTranslation은 레이아웃에 영향 없이 시각적으로만 이동)
		SetRenderTranslation(FVector2D(0.0f, -MoveUpDistance * FadeProgress));
	}
}
