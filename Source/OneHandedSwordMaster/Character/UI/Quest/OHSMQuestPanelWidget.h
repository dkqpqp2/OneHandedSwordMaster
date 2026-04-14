#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"
#include "OHSMQuestPanelWidget.generated.h"

class UOHSMQuestComponent;
class AOHSMQuestNPC;

/**
 * 퀘스트 NPC 와 대화 시 표시되는 퀘스트 패널.
 *
 * ── 왼쪽: 퀘스트 목록 (ScrollBox_QuestList + WBP_QuestEntry)
 * ── 오른쪽: 선택된 퀘스트 상세 정보
 *    - TextDetailName       : 퀘스트 이름
 *    - TextDetailDescription: 설명
 *    - TextDetailObjectives : 목표 목록 (개행 구분)
 *    - TextDetailReward     : 보상 정보
 *    - BtnAccept            : 수락 / 완료 버튼
 *    - TextBtnLabel         : 버튼 라벨 ("수락" or "완료")
 * ── BtnClose               : 닫기 버튼
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMQuestPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** PlayerController 에서 호출 — NPC 와 QuestComponent 연결 */
	void InitializePanel(UOHSMQuestComponent* InQuestComp, AOHSMQuestNPC* InNPC);

	/** 패널 닫기 */
	void ClosePanel();

protected:
	// ── 목록 ──────────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScrollBox> ScrollBox_QuestList;

	// ── 상세 ──────────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextDetailName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextDetailDescription;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextDetailObjectives;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextDetailReward;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> BtnAccept;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextBtnLabel;

	// ── 탭 버튼 ──────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> BtnTab_Available;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> BtnTab_Active;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> BtnTab_Completed;

	// ── 추적 체크박스 ──────────────────────────────────────────────────
	/** 체크 = 추적 중, 해제 = 추적 안 함 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCheckBox> ChkTrack;

	// ── 닫기 ──────────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> BtnClose;

	// ── Entry 위젯 클래스 ─────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	TSubclassOf<class UOHSMQuestEntryWidget> QuestEntryWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UOHSMQuestComponent> QuestComponent;

	UPROPERTY()
	TObjectPtr<AOHSMQuestNPC> CurrentNPC;

	FName SelectedQuestID;

	void BuildQuestList();
	void OnQuestEntryClicked(FName QuestID);
	void RefreshDetail();
	FString BuildObjectivesText(const FOHSMQuestData* Data) const;
	FString BuildRewardText(const FOHSMQuestData* Data) const;

	UFUNCTION() void OnBtnAcceptClicked();
	UFUNCTION() void OnChkTrackChanged(bool bIsChecked);
	UFUNCTION() void OnBtnCloseClicked();
	UFUNCTION() void OnBtnTab_AvailableClicked();
	UFUNCTION() void OnBtnTab_ActiveClicked();
	UFUNCTION() void OnBtnTab_CompletedClicked();

	/** 현재 선택된 탭 (0=진행가능 1=진행중 2=완료됨) */
	int32 CurrentTab = 0;

	void UpdateTabVisuals();
};
