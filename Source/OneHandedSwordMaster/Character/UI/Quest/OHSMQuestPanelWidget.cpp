#include "OHSMQuestPanelWidget.h"

#include "OHSMQuestEntryWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"
#include "OneHandedSwordMaster/NPC/OHSMQuestNPC.h"

// ─── 초기화 ──────────────────────────────────────────────────────────────────

void UOHSMQuestPanelWidget::InitializePanel(UOHSMQuestComponent* InQuestComp, AOHSMQuestNPC* InNPC)
{
	QuestComponent  = InQuestComp;
	CurrentNPC      = InNPC;
	SelectedQuestID = NAME_None;

	CurrentTab = 0;

	// 중복 바인딩 방지 — 기존 바인딩 제거 후 다시 등록
	if (BtnAccept)
	{
		BtnAccept->OnClicked.Clear();
		BtnAccept->OnClicked.AddDynamic(this, &UOHSMQuestPanelWidget::OnBtnAcceptClicked);
	}
	if (BtnClose)
	{
		BtnClose->OnClicked.Clear();
		BtnClose->OnClicked.AddDynamic(this, &UOHSMQuestPanelWidget::OnBtnCloseClicked);
	}
	if (BtnTab_Available)
	{
		BtnTab_Available->OnClicked.Clear();
		BtnTab_Available->OnClicked.AddDynamic(this, &UOHSMQuestPanelWidget::OnBtnTab_AvailableClicked);
	}
	if (BtnTab_Active)
	{
		BtnTab_Active->OnClicked.Clear();
		BtnTab_Active->OnClicked.AddDynamic(this, &UOHSMQuestPanelWidget::OnBtnTab_ActiveClicked);
	}
	if (BtnTab_Completed)
	{
		BtnTab_Completed->OnClicked.Clear();
		BtnTab_Completed->OnClicked.AddDynamic(this, &UOHSMQuestPanelWidget::OnBtnTab_CompletedClicked);
	}
	if (ChkTrack)
	{
		ChkTrack->OnCheckStateChanged.Clear();
		ChkTrack->OnCheckStateChanged.AddDynamic(this, &UOHSMQuestPanelWidget::OnChkTrackChanged);
	}

	UpdateTabVisuals();
	BuildQuestList();
	RefreshDetail();
}

// ─── 목록 빌드 ───────────────────────────────────────────────────────────────

void UOHSMQuestPanelWidget::BuildQuestList()
{
	if (!ScrollBox_QuestList || !QuestComponent || !CurrentNPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestPanel] BuildQuestList: 필수 참조가 null — ScrollBox:%d QuestComp:%d NPC:%d"),
			ScrollBox_QuestList != nullptr, QuestComponent != nullptr, CurrentNPC != nullptr);
		return;
	}

	ScrollBox_QuestList->ClearChildren();

	if (!QuestEntryWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestPanel] QuestEntryWidgetClass 가 설정되지 않았습니다. WBP_QuestPanel Class Defaults 에서 지정하세요."));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestPanel] GetOwningPlayer() 가 null"));
		return;
	}

	for (const FName& QuestID : CurrentNPC->AvailableQuestIDs)
	{
		const FOHSMQuestData* Data = QuestComponent->GetQuestData(QuestID);
		if (!Data)
		{
			UE_LOG(LogTemp, Warning, TEXT("[QuestPanel] QuestID '%s' 를 DataTable 에서 찾을 수 없습니다."), *QuestID.ToString());
			continue;
		}

		EQuestState State = QuestComponent->GetQuestState(QuestID);

		// 탭 필터링
		// 0: 진행 가능 (NotStarted — 선행 퀘스트 완료된 것만)
		// 1: 진행 중   (Active / Completable)
		// 2: 완료됨    (Completed)
		bool bShow = false;
		switch (CurrentTab)
		{
		case 0: bShow = (State == EQuestState::NotStarted) && QuestComponent->CanAcceptQuest(QuestID); 
			break;
		case 1: bShow = (State == EQuestState::Active || State == EQuestState::Completable);            
			break;
		case 2: bShow = (State == EQuestState::Completed);                                             
			break;
		default: 
			break;
		}
		if (!bShow) continue;

		UOHSMQuestEntryWidget* Entry = CreateWidget<UOHSMQuestEntryWidget>(PC, QuestEntryWidgetClass);
		if (!Entry) continue;

		Entry->InitializeEntry(QuestID, Data, State);
		Entry->OnClicked.AddUObject(this, &UOHSMQuestPanelWidget::OnQuestEntryClicked);
		ScrollBox_QuestList->AddChild(Entry);
	}
}

// ─── 퀘스트 선택 ─────────────────────────────────────────────────────────────

void UOHSMQuestPanelWidget::OnQuestEntryClicked(FName QuestID)
{
	SelectedQuestID = QuestID;
	RefreshDetail();
}

// ─── 상세 패널 갱신 ──────────────────────────────────────────────────────────

void UOHSMQuestPanelWidget::RefreshDetail()
{
	if (SelectedQuestID.IsNone() || !QuestComponent)
	{
		if (TextDetailName)        TextDetailName->SetText(FText::FromString(TEXT("퀘스트를 선택하세요")));
		if (TextDetailDescription) TextDetailDescription->SetText(FText::GetEmpty());
		if (TextDetailObjectives)  TextDetailObjectives->SetText(FText::GetEmpty());
		if (TextDetailReward)      TextDetailReward->SetText(FText::GetEmpty());
		if (BtnAccept)             BtnAccept->SetVisibility(ESlateVisibility::Collapsed);
		if (ChkTrack)              ChkTrack->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FOHSMQuestData* Data = QuestComponent->GetQuestData(SelectedQuestID);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestPanel] RefreshDetail: SelectedQuestID '%s' 데이터 없음"), *SelectedQuestID.ToString());
		return;
	}

	EQuestState State = QuestComponent->GetQuestState(SelectedQuestID);

	if (TextDetailName)        TextDetailName->SetText(Data->QuestName);
	if (TextDetailDescription) TextDetailDescription->SetText(Data->Description);
	if (TextDetailObjectives)  TextDetailObjectives->SetText(FText::FromString(BuildObjectivesText(Data)));
	if (TextDetailReward)      TextDetailReward->SetText(FText::FromString(BuildRewardText(Data)));

	if (BtnAccept && TextBtnLabel)
	{
		if (State == EQuestState::NotStarted)
		{
			BtnAccept->SetVisibility(ESlateVisibility::Visible);
			TextBtnLabel->SetText(FText::FromString(TEXT("수락")));
		}
		else if (State == EQuestState::Completable)
		{
			BtnAccept->SetVisibility(ESlateVisibility::Visible);
			TextBtnLabel->SetText(FText::FromString(TEXT("완료")));
		}
		else
		{
			BtnAccept->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 추적 체크박스 — 진행 중 / 완료 가능 퀘스트만 표시
	if (ChkTrack)
	{
		const bool bCanTrack = (State == EQuestState::Active || State == EQuestState::Completable);
		ChkTrack->SetVisibility(bCanTrack ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		if (bCanTrack && QuestComponent)
		{
			// SetIsChecked 가 OnCheckStateChanged 를 트리거하지만
			// TrackQuest / UntrackQuest 내부에서 중복 처리를 막으므로 안전
			ChkTrack->SetIsChecked(QuestComponent->IsQuestTracked(SelectedQuestID));
		}
	}
}

FString UOHSMQuestPanelWidget::BuildObjectivesText(const FOHSMQuestData* Data) const
{
	if (!Data || !QuestComponent) return TEXT("");

	FString Result;
	for (int32 i = 0; i < Data->Objectives.Num(); ++i)
	{
		const FQuestObjectiveData& Obj = Data->Objectives[i];
		const int32 Current  = QuestComponent->GetObjectiveProgress(Data->QuestID, i);
		const int32 Required = Obj.RequiredCount;
		Result += FString::Printf(TEXT("• %s  (%d / %d)\n"), *Obj.Description.ToString(), Current, Required);
	}
	return Result;
}

FString UOHSMQuestPanelWidget::BuildRewardText(const FOHSMQuestData* Data) const
{
	if (!Data) return TEXT("");

	FString Result = TEXT("[보상]\n");
	if (Data->Reward.RewardExp > 0)
		Result += FString::Printf(TEXT("  경험치: %d\n"), Data->Reward.RewardExp);
	if (Data->Reward.RewardSkillPoints > 0)
		Result += FString::Printf(TEXT("  스킬 포인트: %d\n"), Data->Reward.RewardSkillPoints);
	for (int32 i = 0; i < Data->Reward.RewardItemIDs.Num(); ++i)
	{
		const int32 Cnt = Data->Reward.RewardItemCounts.IsValidIndex(i) ? Data->Reward.RewardItemCounts[i] : 1;
		Result += FString::Printf(TEXT("  아이템: %s × %d\n"), *Data->Reward.RewardItemIDs[i].ToString(), Cnt);
	}
	return Result;
}

// ─── 버튼 콜백 ───────────────────────────────────────────────────────────────

void UOHSMQuestPanelWidget::OnBtnAcceptClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[QuestPanel] 수락/완료 버튼 클릭 — QuestID: %s"), *SelectedQuestID.ToString());

	if (!QuestComponent || SelectedQuestID.IsNone()) return;

	const EQuestState State = QuestComponent->GetQuestState(SelectedQuestID);

	if (State == EQuestState::NotStarted)
	{
		QuestComponent->AcceptQuest(SelectedQuestID);
	}
	else if (State == EQuestState::Completable)
	{
		QuestComponent->CompleteQuest(SelectedQuestID);
		SelectedQuestID = NAME_None;
	}

	BuildQuestList();
	RefreshDetail();
}

void UOHSMQuestPanelWidget::OnBtnTab_AvailableClicked()
{
	CurrentTab = 0;
	SelectedQuestID = NAME_None;
	UpdateTabVisuals();
	BuildQuestList();
	RefreshDetail();
}

void UOHSMQuestPanelWidget::OnBtnTab_ActiveClicked()
{
	CurrentTab = 1;
	SelectedQuestID = NAME_None;
	UpdateTabVisuals();
	BuildQuestList();
	RefreshDetail();
}

void UOHSMQuestPanelWidget::OnBtnTab_CompletedClicked()
{
	CurrentTab = 2;
	SelectedQuestID = NAME_None;
	UpdateTabVisuals();
	BuildQuestList();
	RefreshDetail();
}

void UOHSMQuestPanelWidget::UpdateTabVisuals()
{
	// 선택된 탭은 밝게, 나머지는 어둡게
	const FLinearColor ActiveColor   = FLinearColor(1.f, 0.8f, 0.2f); // 노란색
	const FLinearColor InactiveColor = FLinearColor(0.3f, 0.3f, 0.3f);

	if (BtnTab_Available)
		BtnTab_Available->SetBackgroundColor(CurrentTab == 0 ? ActiveColor : InactiveColor);
	if (BtnTab_Active)
		BtnTab_Active->SetBackgroundColor(CurrentTab == 1 ? ActiveColor : InactiveColor);
	if (BtnTab_Completed)
		BtnTab_Completed->SetBackgroundColor(CurrentTab == 2 ? ActiveColor : InactiveColor);
}

void UOHSMQuestPanelWidget::OnChkTrackChanged(bool bIsChecked)
{
	if (!QuestComponent || SelectedQuestID.IsNone()) return;

	if (bIsChecked)
	{
		QuestComponent->TrackQuest(SelectedQuestID);
	}
	else
	{
		QuestComponent->UntrackQuest(SelectedQuestID);
	}
}

void UOHSMQuestPanelWidget::OnBtnCloseClicked()
{
	ClosePanel();
}

void UOHSMQuestPanelWidget::ClosePanel()
{
	// AOHSMPlayerController::CloseQuestPanel() 을 통해 닫아야
	// bIsQuestPanelOpen 플래그가 정확히 해제되고 IsAnyWindowOpen() 이 false 를 반환함
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(GetOwningPlayer()))
	{
		PC->CloseQuestPanel();
	}
	else
	{
		// 폴백 (컨트롤러 캐스트 실패 시)
		SetVisibility(ESlateVisibility::Collapsed);
		if (APlayerController* FallbackPC = GetOwningPlayer())
		{
			FInputModeGameOnly GameOnly;
			FallbackPC->SetInputMode(GameOnly);
			FallbackPC->SetShowMouseCursor(false);
		}
	}
}
