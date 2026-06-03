#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMTrackerEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UOHSMQuestComponent;

/** 자동이동 버튼 클릭 시 QuestID 전달 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTrackerEntryAutoMove, FName /*QuestID*/);

// 퀘스트 트래커 개별 행 위젯
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMTrackerEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 엔트리 초기화 (퀘스트 ID, 완료 여부)
	void InitEntry(UOHSMQuestComponent* InQuestComp, FName InQuestID, bool bCompleted);

	// 자동이동 요청 델리게이트
	FOnTrackerEntryAutoMove OnAutoMoveRequested;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextQuestName;

	// 퀘스트 설명 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextDescription;

	// 목표 진행도 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextObjectives;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BtnAutoMove;

	// 자동이동 버튼 내 텍스트 (WBP에서 이름을 "TextBtnAutoMove" 로 맞춰야 함)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBtnAutoMove;

public:
	// 버튼 텍스트를 현재 자동이동 상태에 맞게 갱신
	void RefreshAutoMoveButton();

private:
	FName CachedQuestID;

	UFUNCTION()
	void OnBtnAutoMoveClicked();
};
