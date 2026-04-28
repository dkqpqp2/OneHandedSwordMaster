#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMNPCInfoWidget.generated.h"

/**
 * NPC 머리 위에 표시되는 정보 위젯.
 * - TextNPCName        (BindWidgetOptional) : NPC 이름
 * - TextQuestIndicator (BindWidgetOptional) : 퀘스트 인디케이터 (! / ?)
 *
 * [인디케이터 규칙]
 *  ! (주황) — 수락 가능한 퀘스트 있음
 *  ? (노랑) — 완료 가능한 퀘스트 있음 (우선 표시)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMNPCInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** NPC 이름 설정 */
	void SetNPCName(const FText& InName);

	/** 퀘스트 인디케이터 갱신. Completable 이 Available 보다 우선 */
	void SetQuestIndicator(bool bAvailable, bool bCompletable);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextNPCName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextQuestIndicator;
};
