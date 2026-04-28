#include "OHSMQuestNPC.h"

#include "OHSMNPCInfoWidget.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"

AOHSMQuestNPC::AOHSMQuestNPC()
{
	DialogueMessage   = FText::FromString(TEXT("무엇을 도와드릴까요?"));
	ConfirmButtonText = FText::FromString(TEXT("퀘스트 확인"));
	CancelButtonText  = FText::FromString(TEXT("아니요"));
}

// ─── BeginPlay ───────────────────────────────────────────────────────────────

void AOHSMQuestNPC::BeginPlay()
{
	Super::BeginPlay();

	// 한 프레임 뒤에 구독 — 플레이어 스폰 완료를 보장
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		SubscribeToQuestChanges();
	});
}

void AOHSMQuestNPC::SubscribeToQuestChanges()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC) return;

	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(PC->GetPawn());
	if (!Player) return;

	CachedQuestComponent = Player->GetQuestComponent();
	if (!CachedQuestComponent) return;

	// 퀘스트 상태가 바뀔 때마다 인디케이터 갱신
	CachedQuestComponent->OnQuestAccepted.AddUObject(this, &AOHSMQuestNPC::OnQuestStateChanged);
	CachedQuestComponent->OnQuestCompleted.AddUObject(this, &AOHSMQuestNPC::OnQuestStateChanged);
	CachedQuestComponent->OnQuestProgressUpdated.AddUObject(this, &AOHSMQuestNPC::OnQuestStateChanged);

	UpdateQuestIndicator();
}

// ─── 인디케이터 갱신 ─────────────────────────────────────────────────────────

void AOHSMQuestNPC::OnQuestStateChanged(FName /*QuestID*/)
{
	UpdateQuestIndicator();
}

void AOHSMQuestNPC::UpdateQuestIndicator()
{
	UOHSMNPCInfoWidget* InfoWidget = GetInfoWidget();
	if (!InfoWidget || !CachedQuestComponent) return;

	bool bAvailable   = false;
	bool bCompletable = false;

	for (const FName& QuestID : AvailableQuestIDs)
	{
		const EQuestState State = CachedQuestComponent->GetQuestState(QuestID);

		if (State == EQuestState::Completable)
		{
			bCompletable = true;
			break; // 완료 가능이 최우선 — 더 볼 필요 없음
		}
		if (State == EQuestState::NotStarted && CachedQuestComponent->CanAcceptQuest(QuestID))
		{
			bAvailable = true;
		}
	}

	InfoWidget->SetQuestIndicator(bAvailable, bCompletable);
}

// ─── 대화 확인 ────────────────────────────────────────────────────────────────

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

	// AvailableQuestIDs 가 있는 NPC 만 패널 오픈
	// (대화 타겟 전용 NPC 는 AvailableQuestIDs 를 비워두면 패널이 열리지 않음)
	if (AvailableQuestIDs.Num() > 0)
	{
		if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(InstigatorController))
		{
			PC->OpenQuestPanel(this);
		}
	}
}
