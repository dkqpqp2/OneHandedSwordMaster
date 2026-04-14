#include "OHSMQuestTrackerWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"

// ─── 초기화 ──────────────────────────────────────────────────────────────────

void UOHSMQuestTrackerWidget::InitializeTracker(UOHSMQuestComponent* InQuestComp)
{
	QuestComponent = InQuestComp;
	if (!QuestComponent) return;

	// 진행도 변경 시 텍스트 갱신
	QuestComponent->OnQuestProgressUpdated.AddUObject(this, &UOHSMQuestTrackerWidget::OnQuestProgressUpdated);

	// 추적 목록 변경 시 전체 갱신 (TrackQuest / UntrackQuest / CompleteQuest)
	QuestComponent->OnQuestTrackingChanged.AddUObject(this, &UOHSMQuestTrackerWidget::OnTrackingChanged);

	if (BtnToggle)
	{
		BtnToggle->OnClicked.AddDynamic(this, &UOHSMQuestTrackerWidget::OnBtnToggleClicked);
	}

	Refresh();
}

// ─── 델리게이트 콜백 ────────────────────────────────────────────────────────

void UOHSMQuestTrackerWidget::OnQuestProgressUpdated(FName /*QuestID*/)
{
	Refresh();
}

void UOHSMQuestTrackerWidget::OnTrackingChanged()
{
	Refresh();
}

void UOHSMQuestTrackerWidget::OnBtnToggleClicked()
{
	bIsExpanded = !bIsExpanded;

	if (BoxContent)
	{
		BoxContent->SetVisibility(
			bIsExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed
		);
	}
}

// ─── 갱신 ────────────────────────────────────────────────────────────────────

void UOHSMQuestTrackerWidget::Refresh()
{
	if (!QuestComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const TArray<FName>& TrackedIDs = QuestComponent->GetTrackedQuestIDs();
	if (TrackedIDs.Num() == 0)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 추적 퀘스트가 있으면 표시 (SelfHitTestInvisible: 배경은 클릭 통과, 자식 버튼은 동작)
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (!TextTrackerContent) return;

	FString Content;
	for (const FName& QuestID : TrackedIDs)
	{
		const FOHSMQuestData* Data = QuestComponent->GetQuestData(QuestID);
		if (!Data) continue;

		// 퀘스트 이름
		Content += FString::Printf(TEXT("▶ %s\n"), *Data->QuestName.ToString());

		// 목표별 진행도
		for (int32 i = 0; i < Data->Objectives.Num(); ++i)
		{
			const FQuestObjectiveData& Obj = Data->Objectives[i];
			const int32 Current  = QuestComponent->GetObjectiveProgress(QuestID, i);
			const int32 Required = Obj.RequiredCount;
			const bool  bDone    = (Current >= Required);

			Content += FString::Printf(TEXT("  %s %s  %d/%d\n"),
				bDone ? TEXT("✓") : TEXT("○"),
				*Obj.Description.ToString(),
				Current, Required);
		}

		Content += TEXT("\n");
	}

	TextTrackerContent->SetText(FText::FromString(Content.TrimEnd()));
}
