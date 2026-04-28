#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMTrackerEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UOHSMQuestComponent;

/** 자동이동 버튼 클릭 시 QuestID 전달 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTrackerEntryAutoMove, FName /*QuestID*/);

/**
 * 퀘스트 트래커 내 개별 퀘스트 행 위젯.
 *
 * ── WBP 구성 요소 ─────────────────────────────────────────────
 *  TextQuestName  (BindWidget)         : 퀘스트 이름 (색상으로 상태 표시)
 *  TextObjectives (BindWidgetOptional) : 목표 진행도 (완료된 퀘스트는 숨김)
 *  BtnAutoMove    (BindWidgetOptional) : "➤ 이동" 버튼 (완료된 퀘스트는 숨김)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMTrackerEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 엔트리 초기화.
	 * @param InQuestComp   플레이어의 QuestComponent
	 * @param InQuestID     표시할 퀘스트 ID
	 * @param bCompleted    true 이면 완료 스타일로 렌더링 (이동 버튼 숨김)
	 */
	void InitEntry(UOHSMQuestComponent* InQuestComp, FName InQuestID, bool bCompleted);

	/** BtnAutoMove 클릭 시 브로드캐스트 */
	FOnTrackerEntryAutoMove OnAutoMoveRequested;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextQuestName;

	/** 퀘스트 설명 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextDescription;

	/** 목표 진행도 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextObjectives;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BtnAutoMove;

private:
	FName CachedQuestID;

	UFUNCTION()
	void OnBtnAutoMoveClicked();
};
