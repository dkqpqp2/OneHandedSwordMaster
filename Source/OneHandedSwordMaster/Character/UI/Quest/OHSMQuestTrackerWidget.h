#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMQuestTrackerWidget.generated.h"

class UOHSMQuestComponent;

/**
 * 화면 우측에 고정 표시되는 퀘스트 트래커.
 *
 * ── WBP 구성 요소 ──────────────────────────────────────────────
 * - BtnToggle         (BindWidgetOptional) : 접기 / 펼치기 버튼
 * - BoxContent        (BindWidgetOptional) : 접기 / 펼치기 대상 컨테이너
 * - TextTrackerContent (BindWidget)        : 추적 퀘스트 목록 텍스트
 *
 * TrackQuest / UntrackQuest 호출 시 OnQuestTrackingChanged 를 받아 자동 갱신.
 * 추적 퀘스트가 없으면 위젯 전체를 Collapsed.
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMQuestTrackerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** HUD 초기화 시 호출 */
	void InitializeTracker(UOHSMQuestComponent* InQuestComp);

	/** 외부에서 강제 갱신 시 호출 */
	void Refresh();

protected:
	/** 접기 / 펼치기 버튼 (클릭하면 BoxContent 토글) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> BtnToggle;

	/** 접기 / 펼치기 대상 컨테이너 (SizeBox, VerticalBox 등) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UWidget> BoxContent;

	/** 모든 추적 퀘스트의 이름 + 목표 진행도를 하나의 텍스트로 표시 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextTrackerContent;

private:
	UPROPERTY()
	TObjectPtr<UOHSMQuestComponent> QuestComponent;

	bool bIsExpanded = true;

	void OnQuestProgressUpdated(FName QuestID);
	void OnTrackingChanged();

	UFUNCTION()
	void OnBtnToggleClicked();
};
