#include "OHSMTrackerEntryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"

void UOHSMTrackerEntryWidget::InitEntry(UOHSMQuestComponent* InQuestComp, FName InQuestID, bool bCompleted)
{
	CachedQuestID = InQuestID;

	if (!InQuestComp)
	{
		return;
	}

	const FOHSMQuestData* Data = InQuestComp->GetQuestData(InQuestID);
	if (!Data)
	{
		return;
	}

	// ── 퀘스트 이름 ──────────────────────────────────────────────────────────
	if (TextQuestName)
	{
		FLinearColor NameColor;
		FString Prefix;

		if (bCompleted)
		{
			NameColor = FLinearColor(0.4f, 0.75f, 1.0f); // 하늘색
			Prefix    = TEXT("✓ ");
		}
		else
		{
			EQuestState State = InQuestComp->GetQuestState(InQuestID);
			if (State == EQuestState::Completable)
			{
				NameColor = FLinearColor(0.2f, 1.0f, 0.2f); // 초록 (완료 가능)
				Prefix    = TEXT("▶ ");
			}
			else
			{
				NameColor = FLinearColor(1.0f, 0.9f, 0.3f); // 노란색 (진행 중)
				Prefix    = TEXT("▶ ");
			}
		}

		TextQuestName->SetText(FText::FromString(Prefix + Data->QuestName.ToString()));
		TextQuestName->SetColorAndOpacity(FSlateColor(NameColor));
	}

	// ── 퀘스트 설명 ─────────────────────────────────────────────────────────
	if (TextDescription)
	{
		TextDescription->SetText(Data->Description);
		TextDescription->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)));
		TextDescription->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// ── 목표 진행도 ─────────────────────────────────────────────────────────
	if (TextObjectives)
	{
		if (!bCompleted)
		{
			FString ObjText;
			for (int32 i = 0; i < Data->Objectives.Num(); ++i)
			{
				const FQuestObjectiveData& Obj = Data->Objectives[i];
				const int32 Current  = InQuestComp->GetObjectiveProgress(InQuestID, i);
				const int32 Required = Obj.RequiredCount;
				const bool  bDone    = (Current >= Required);

				ObjText += FString::Printf(TEXT("  %s %s  %d/%d\n"),
					bDone ? TEXT("✓") : TEXT("○"),
					*Obj.Description.ToString(),
					Current, Required);
			}
			TextObjectives->SetText(FText::FromString(ObjText.TrimEnd()));
			TextObjectives->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			TextObjectives->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// ── 자동이동 버튼 ────────────────────────────────────────────────────────
	if (BtnAutoMove)
	{
		if (!bCompleted)
		{
			BtnAutoMove->OnClicked.Clear();
			BtnAutoMove->OnClicked.AddDynamic(this, &UOHSMTrackerEntryWidget::OnBtnAutoMoveClicked);
			BtnAutoMove->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BtnAutoMove->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UOHSMTrackerEntryWidget::OnBtnAutoMoveClicked()
{
	OnAutoMoveRequested.Broadcast(CachedQuestID);
}
