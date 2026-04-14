#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"
#include "OHSMQuestEntryWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestEntryClicked, FName /*QuestID*/);

/**
 * 퀘스트 목록의 단일 항목 위젯.
 * - TextQuestName : 퀘스트 이름 (상태에 따라 색상 변경)
 * - TextStatus    : 선택적 컬러 인디케이터 (● 점 — BindWidgetOptional)
 * - BtnEntry      : 클릭 시 OnClicked 브로드캐스트
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeEntry(FName InQuestID, const FOHSMQuestData* InData, EQuestState InState);

	FName GetQuestID() const { return QuestID; }

	FOnQuestEntryClicked OnClicked;

protected:
	/** 퀘스트 이름 — 상태에 따라 색상 변경 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextQuestName;

	/** 선택적 상태 인디케이터 (● 점). WBP에 없어도 빌드 오류 없음 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextStatus;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> BtnEntry;

private:
	FName QuestID;

	UFUNCTION()
	void OnBtnClicked();
};
