#include "OHSMNPCInfoWidget.h"

#include "Components/TextBlock.h"

void UOHSMNPCInfoWidget::SetNPCName(const FText& InName)
{
	if (TextNPCName)
	{
		TextNPCName->SetText(InName);
	}
}

void UOHSMNPCInfoWidget::SetQuestIndicator(bool bAvailable, bool bCompletable)
{
	if (!TextQuestIndicator) return;

	if (bCompletable)
	{
		// ? — 완료 가능 (노랑, 우선)
		TextQuestIndicator->SetText(FText::FromString(TEXT("?")));
		TextQuestIndicator->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.9f, 0.1f)));
		TextQuestIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else if (bAvailable)
	{
		// ! — 수락 가능 (주황)
		TextQuestIndicator->SetText(FText::FromString(TEXT("!")));
		TextQuestIndicator->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.55f, 0.f)));
		TextQuestIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		// 관련 퀘스트 없음 — 숨김
		TextQuestIndicator->SetVisibility(ESlateVisibility::Collapsed);
	}
}
