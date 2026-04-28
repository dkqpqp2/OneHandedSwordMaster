#include "OHSMDeathWidget.h"

#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOHSMDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BtnRevivalPotion)
	{
		BtnRevivalPotion->OnClicked.AddDynamic(this, &UOHSMDeathWidget::OnRevivalButtonClicked);
	}

	if (BtnRespawnAtTown)
	{
		BtnRespawnAtTown->OnClicked.AddDynamic(this, &UOHSMDeathWidget::OnRespawnAtTownButtonClicked);
	}
}

void UOHSMDeathWidget::UpdateCountdown(float Remaining, float Total)
{
	if (TextCountdown)
	{
		int32 Seconds = FMath::CeilToInt(Remaining);
		TextCountdown->SetText(
			FText::FromString(FString::Printf(TEXT("%d초 후 마을에서 자동 부활"), Seconds))
		);
	}

	if (ProgressCountdown && Total > 0.f)
	{
		ProgressCountdown->SetPercent(Remaining / Total);
	}
}

void UOHSMDeathWidget::UpdateRevivalButton(int32 PotionCount)
{
	if (BtnRevivalPotion)
	{
		// 항상 표시, 포션 없으면 버튼 비활성화(회색)
		BtnRevivalPotion->SetVisibility(ESlateVisibility::Visible);
		BtnRevivalPotion->SetIsEnabled(PotionCount > 0);
	}

	if (TextPotionCount)
	{
		TextPotionCount->SetText(
			FText::FromString(FString::Printf(TEXT("%d개"), PotionCount))
		);
	}
}

void UOHSMDeathWidget::OnRevivalButtonClicked()
{
	OnRevivalPotionRequested.Broadcast();
}

void UOHSMDeathWidget::OnRespawnAtTownButtonClicked()
{
	OnRespawnAtTownRequested.Broadcast();
}
