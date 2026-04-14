#pragma once

#include "CoreMinimal.h"
#include "OHSMNPCBase.h"
#include "OHSMQuestNPC.generated.h"

/**
 * 퀘스트를 부여하는 NPC.
 * - AvailableQuestIDs: 이 NPC가 줄 수 있는 퀘스트 목록 (에디터에서 설정)
 * - 상호작용 시 WBP_QuestPanel 을 열어 수락/완료 처리
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMQuestNPC : public AOHSMNPCBase
{
	GENERATED_BODY()

public:
	AOHSMQuestNPC();

	/** 이 NPC가 제공하는 퀘스트 ID 목록 (DT_QuestData Row Name) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FName> AvailableQuestIDs;

	/** NPC 고유 ID — Talk 목표의 TargetID 와 매칭 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName NPC_ID;

protected:
	/** 확인 버튼 → 퀘스트 패널 오픈 */
	virtual void OnDialogueConfirmed(APlayerController* InstigatorController) override;
};
