#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMQuestTrackerWidget.generated.h"

class UOHSMQuestComponent;
class UOHSMTrackerEntryWidget;
class UButton;
class UWidget;
class UVerticalBox;

/**
 * 화면 우측에 고정 표시되는 퀘스트 트래커.
 *
 * ── WBP 구성 요소 ──────────────────────────────────────────────
 *  BtnToggle        (BindWidgetOptional) : 접기/펼치기 버튼
 *  BoxContent       (BindWidgetOptional) : 접기/펼치기 대상 컨테이너
 *  BoxTrackedQuests (BindWidget)         : 추적 퀘스트 엔트리 동적 채움
 *
 * ── Class Defaults 에서 설정 ───────────────────────────────────
 *  TrackerEntryClass : WBP_TrackerEntry 클래스 지정
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMQuestTrackerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeTracker(UOHSMQuestComponent* InQuestComp);
	void Refresh();

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BtnToggle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BoxContent;

	/** 추적 중인 퀘스트 엔트리가 채워지는 컨테이너 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> BoxTrackedQuests;

	/** 엔트리 위젯 클래스 — BP에서 WBP_TrackerEntry 로 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Quest|Tracker")
	TSubclassOf<UOHSMTrackerEntryWidget> TrackerEntryClass;

private:
	UPROPERTY()
	TObjectPtr<UOHSMQuestComponent> QuestComponent;

	bool bIsExpanded = true;

	void OnQuestProgressUpdated(FName QuestID);
	void OnTrackingChanged();

	UFUNCTION()
	void OnBtnToggleClicked();

	void HandleAutoMove(FName QuestID);
};
