#include "OHSMQuestNPC.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"

AOHSMQuestNPC::AOHSMQuestNPC()
{
	DialogueMessage   = FText::FromString(TEXT("무엇을 도와드릴까요?"));
	ConfirmButtonText = FText::FromString(TEXT("퀘스트 확인"));
	CancelButtonText  = FText::FromString(TEXT("아니요"));
}

void AOHSMQuestNPC::OnDialogueConfirmed(APlayerController* InstigatorController)
{
	UE_LOG(LogTemp, Log, TEXT("[QuestNPC] OnDialogueConfirmed 호출됨 — NPC: %s"), *GetName());

	// Talk 목표 진행도 업데이트
	if (!NPC_ID.IsNone())
	{
		if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(InstigatorController->GetPawn()))
		{
			if (UOHSMQuestComponent* QuestComp = Player->GetQuestComponent())
			{
				QuestComp->NotifyTalkedToNPC(NPC_ID);
			}
		}
	}

	// 퀘스트 패널 오픈
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(InstigatorController))
	{
		PC->OpenQuestPanel(this);
	}
}
