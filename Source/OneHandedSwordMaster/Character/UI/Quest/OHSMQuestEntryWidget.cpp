#include "OHSMQuestEntryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UOHSMQuestEntryWidget::InitializeEntry(FName InQuestID, const FOHSMQuestData* InData, EQuestState InState)
{
	QuestID = InQuestID;

	if (TextQuestName && InData)
	{
		TextQuestName->SetText(InData->QuestName);

		// 상태에 따라 퀘스트 이름 색상 변경 (텍스트만으로 상태 구분)
		FLinearColor NameColor;
		switch (InState)
		{
		case EQuestState::Completable: NameColor = FLinearColor(0.2f, 1.0f, 0.2f); break; // 초록 — 완료 가능
		case EQuestState::Active:      NameColor = FLinearColor(1.0f, 0.9f, 0.3f); break; // 노랑 — 진행 중
		case EQuestState::Completed:   NameColor = FLinearColor(0.4f, 0.75f, 1.0f); break; // 하늘색 — 완료됨
		default:                       NameColor = FLinearColor::White;             break; // 흰색 — 수락 가능
		}
		TextQuestName->SetColorAndOpacity(FSlateColor(NameColor));
	}

	// TextStatus 가 WBP에 있으면 컬러 점(●)으로 활용
	if (TextStatus)
	{
		switch (InState)
		{
		case EQuestState::Completable:
			TextStatus->SetText(FText::FromString(TEXT("●")));
			TextStatus->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.0f, 0.2f)));
			TextStatus->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;
		case EQuestState::Active:
			TextStatus->SetText(FText::FromString(TEXT("●")));
			TextStatus->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.3f)));
			TextStatus->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;
		case EQuestState::Completed:
			TextStatus->SetText(FText::FromString(TEXT("●")));
			TextStatus->SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.75f, 1.0f)));
			TextStatus->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;
		default:
			TextStatus->SetVisibility(ESlateVisibility::Hidden);
			break;
		}
	}

	if (BtnEntry)
	{
		BtnEntry->OnClicked.AddDynamic(this, &UOHSMQuestEntryWidget::OnBtnClicked);
	}
}

void UOHSMQuestEntryWidget::OnBtnClicked()
{
	OnClicked.Broadcast(QuestID);
}
