#include "OHSMQuestTrackerWidget.h"

#include "OHSMTrackerEntryWidget.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestNavigationComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"

// ─── 초기화 ──────────────────────────────────────────────────────────────────

void UOHSMQuestTrackerWidget::InitializeTracker(UOHSMQuestComponent* InQuestComp)
{
	QuestComponent = InQuestComp;
	if (!QuestComponent)
	{
		return;
	}

	QuestComponent->OnQuestProgressUpdated.AddUObject(this, &UOHSMQuestTrackerWidget::OnQuestProgressUpdated);
	QuestComponent->OnQuestTrackingChanged.AddUObject(this, &UOHSMQuestTrackerWidget::OnTrackingChanged);

	if (BtnToggle)
	{
		BtnToggle->OnClicked.AddDynamic(this, &UOHSMQuestTrackerWidget::OnBtnToggleClicked);
	}

	Refresh();
}

// ─── 델리게이트 콜백 ─────────────────────────────────────────────────────────

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
			bIsExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
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
	if (TrackedIDs.IsEmpty())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (!BoxTrackedQuests)
	{
		return;
	}

	BoxTrackedQuests->ClearChildren();

	APlayerController* PC = GetOwningPlayer();
	if (!TrackerEntryClass || !PC)
	{
		return;
	}

	for (const FName& QuestID : TrackedIDs)
	{
		UOHSMTrackerEntryWidget* Entry =
			CreateWidget<UOHSMTrackerEntryWidget>(PC, TrackerEntryClass);
		if (!Entry)
		{
			continue;
		}

		Entry->InitEntry(QuestComponent, QuestID, /*bCompleted=*/false);
		Entry->OnAutoMoveRequested.AddUObject(this, &UOHSMQuestTrackerWidget::HandleAutoMove);
		BoxTrackedQuests->AddChild(Entry);
	}
}

// ─── 자동이동 ────────────────────────────────────────────────────────────────

void UOHSMQuestTrackerWidget::HandleAutoMove(FName QuestID)
{
	AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(GetOwningPlayer());
	if (!PC) return;

	// 이미 자동이동 중이면 중지
	if (PC->IsAutoMoving())
	{
		PC->StopAutoMove();
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	UOHSMQuestNavigationComponent* NavComp =
		Pawn->FindComponentByClass<UOHSMQuestNavigationComponent>();

	FVector TargetLocation;
	bool bFoundTarget = false;

	if (NavComp)
	{
		NavComp->SetManualNavTarget(QuestID);
		bFoundTarget = NavComp->FindQuestTargetLocation(QuestID, TargetLocation);
	}

	if (bFoundTarget)
	{
		PC->StartAutoMoveTo(TargetLocation);
	}
}
