#include "OHSMQuestComponent.h"

#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"

UOHSMQuestComponent::UOHSMQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOHSMQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UOHSMSkillComponent* SkillComp = GetOwner()->FindComponentByClass<UOHSMSkillComponent>())
	{
		SkillComp->OnSkillLearned.AddUObject(this, &UOHSMQuestComponent::NotifySkillLearned);
	}
	
	if (UOHSMInventoryComponent* InvComp = GetOwner()->FindComponentByClass<UOHSMInventoryComponent>())
	{
		InvComp->OnItemAdded.AddDynamic(this, &UOHSMQuestComponent::NotifyItemCollected);
	}
}

bool UOHSMQuestComponent::CanAcceptQuest(FName QuestID) const
{
	if (QuestID.IsNone() || !QuestDataTable) return false;

	// 이미 진행 중이거나 완료됨
	if (IsQuestActive(QuestID) || IsQuestCompleted(QuestID)) return false;

	const FOHSMQuestData* Data = GetQuestData(QuestID);
	if (!Data) return false;

	// 선행 퀘스트 확인 — 모두 완료돼야 수락 가능
	for (const FName& PrereqID : Data->PrerequisiteQuestIDs)
	{
		if (!PrereqID.IsNone() && !IsQuestCompleted(PrereqID))
		{
			return false;
		}
	}

	return true;
}

// 퀘스트 수락 처리
bool UOHSMQuestComponent::AcceptQuest(FName QuestID)
{
	if (!CanAcceptQuest(QuestID)) return false;

	const FOHSMQuestData* Data = GetQuestData(QuestID);
	if (!Data) return false;

	FQuestProgress Progress;
	Progress.Counts.SetNum(Data->Objectives.Num(), false); // 목표 개수만큼 초기화
	for (int32& C : Progress.Counts) C = 0;

	// Collect 목표: 수락 시점에 이미 인벤토리에 있는 아이템 수량 반영
	bool bPreFilled = false;
	if (UOHSMInventoryComponent* InvComp = GetOwner()->FindComponentByClass<UOHSMInventoryComponent>())
	{
		for (int32 i = 0; i < Data->Objectives.Num(); ++i)
		{
			const FQuestObjectiveData& Obj = Data->Objectives[i];
			if (Obj.ObjectiveType != EQuestObjectiveType::Collect) continue;

			const int32 CurrentCount = InvComp->GetItemCount(Obj.TargetID);
			if (CurrentCount > 0)
			{
				Progress.Counts[i] = FMath::Min(CurrentCount, Obj.RequiredCount);
				bPreFilled = true;
			}
		}
	}

	ActiveQuestProgress.Add(QuestID, Progress);
	OnQuestAccepted.Broadcast(QuestID);

	// 기존 아이템이 반영된 경우 UI 갱신
	if (bPreFilled)
	{
		OnQuestProgressUpdated.Broadcast(QuestID);
	}

	// 수락과 동시에 자동 추적 (TrackedQuestIDs 최대 3개)
	TrackQuest(QuestID);

	return true;
}

bool UOHSMQuestComponent::CanCompleteQuest(FName QuestID) const
{
	if (!IsQuestActive(QuestID)) return false;

	const FOHSMQuestData* Data = GetQuestData(QuestID);
	if (!Data) return false;

	const FQuestProgress& Progress = ActiveQuestProgress[QuestID];
	for (int32 i = 0; i < Data->Objectives.Num(); ++i)
	{
		const int32 Current = Progress.Counts.IsValidIndex(i) ? Progress.Counts[i] : 0;
		if (Current < Data->Objectives[i].RequiredCount) return false;
	}

	return true;
}

// 퀘스트 완료 처리
void UOHSMQuestComponent::CompleteQuest(FName QuestID)
{
	if (!CanCompleteQuest(QuestID)) return;

	const FOHSMQuestData* Data = GetQuestData(QuestID);
	if (!Data) return;

	GiveRewards(Data->Reward); // 보상 지급

	ActiveQuestProgress.Remove(QuestID); // 진행 목록 제거
	CompletedQuests.Add(QuestID); // 완료 목록에 추가
	TrackedQuestIDs.Remove(QuestID); // 완료된 퀘스트는 추적 해제

	OnQuestCompleted.Broadcast(QuestID);
	OnQuestTrackingChanged.Broadcast();
}

void UOHSMQuestComponent::NotifyEnemyKilled(FName EnemyID)
{
	UpdateProgress(EQuestObjectiveType::Kill, EnemyID, 1);
}

void UOHSMQuestComponent::NotifyItemCollected(FName ItemID, int32 Count)
{
	UpdateProgress(EQuestObjectiveType::Collect, ItemID, Count);
}

void UOHSMQuestComponent::NotifyTalkedToNPC(FName NPCID)
{
	UpdateProgress(EQuestObjectiveType::Talk, NPCID, 1);
}

void UOHSMQuestComponent::NotifySkillLearned(FName SkillID)
{
	UpdateProgress(EQuestObjectiveType::LearnSkill, SkillID, 1);
}

void UOHSMQuestComponent::UpdateProgress(EQuestObjectiveType Type, FName TargetID, int32 Count)
{
	bool bAnyUpdated = false;

	for (auto& Pair : ActiveQuestProgress)
	{
		const FName& QuestID = Pair.Key;
		FQuestProgress& Progress = Pair.Value;
		const FOHSMQuestData* Data = GetQuestData(QuestID);
		if (!Data) continue;

		for (int32 i = 0; i < Data->Objectives.Num(); ++i)
		{
			const FQuestObjectiveData& Obj = Data->Objectives[i];
			if (Obj.ObjectiveType != Type || Obj.TargetID != TargetID) continue;
			
			if (Progress.Counts[i] >= Obj.RequiredCount) continue;

			Progress.Counts[i] = FMath::Min(Progress.Counts[i] + Count, Obj.RequiredCount);
			bAnyUpdated = true;
		}

		if (bAnyUpdated)
		{
			OnQuestProgressUpdated.Broadcast(QuestID);
			bAnyUpdated = false;
		}
	}
}

// ─── 보상 ────────────────────────────────────────────────────────────────────

// 퀘스트 보상 지급
void UOHSMQuestComponent::GiveRewards(const FQuestRewardData& Reward)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 경험치 지급
	if (Reward.RewardExp > 0)
	{
		if (UOHSMPlayerStatComponent* StatComp = Owner->FindComponentByClass<UOHSMPlayerStatComponent>())
		{
			StatComp->AddExperience(Reward.RewardExp);
		}
	}

	// 스킬 포인트
	if (Reward.RewardSkillPoints > 0)
	{
		if (UOHSMSkillComponent* SkillComp = Owner->FindComponentByClass<UOHSMSkillComponent>())
		{
			SkillComp->AddSkillPoints(Reward.RewardSkillPoints);
		}
	}

	// 아이템
	if (UOHSMInventoryComponent* InvComp = Owner->FindComponentByClass<UOHSMInventoryComponent>())
	{
		for (int32 i = 0; i < Reward.RewardItemIDs.Num(); ++i)
		{
			const int32 Cnt = Reward.RewardItemCounts.IsValidIndex(i) ? Reward.RewardItemCounts[i] : 1;
			InvComp->AddItem(Reward.RewardItemIDs[i], Cnt);
		}
	}
}

// ─── 조회 ────────────────────────────────────────────────────────────────────

EQuestState UOHSMQuestComponent::GetQuestState(FName QuestID) const
{
	if (IsQuestCompleted(QuestID)) return EQuestState::Completed;
	if (IsQuestActive(QuestID))
	{
		return CanCompleteQuest(QuestID) ? EQuestState::Completable : EQuestState::Active;
	}
	return EQuestState::NotStarted;
}

int32 UOHSMQuestComponent::GetObjectiveProgress(FName QuestID, int32 ObjectiveIndex) const
{
	if (const FQuestProgress* P = ActiveQuestProgress.Find(QuestID))
	{
		return P->Counts.IsValidIndex(ObjectiveIndex) ? P->Counts[ObjectiveIndex] : 0;
	}
	return 0;
}

const FOHSMQuestData* UOHSMQuestComponent::GetQuestData(FName QuestID) const
{
	if (!QuestDataTable) return nullptr;
	return QuestDataTable->FindRow<FOHSMQuestData>(QuestID, TEXT("GetQuestData"));
}

TArray<FName> UOHSMQuestComponent::GetActiveQuestIDs() const
{
	TArray<FName> Result;
	ActiveQuestProgress.GetKeys(Result);
	return Result;
}

void UOHSMQuestComponent::TrackQuest(FName QuestID)
{
	if (QuestID.IsNone() || TrackedQuestIDs.Contains(QuestID)) return;
	if (TrackedQuestIDs.Num() >= 3) TrackedQuestIDs.RemoveAt(0); // 최대 3개
	TrackedQuestIDs.Add(QuestID);
	OnQuestTrackingChanged.Broadcast();
}

void UOHSMQuestComponent::RestoreFromSave(
	const TArray<FQuestSaveEntry>& ActiveQuestSaves,
	const TArray<FName>&           InCompletedQuests,
	const TArray<FName>&           InTrackedQuests)
{
	ActiveQuestProgress.Empty();
	CompletedQuests.Empty();
	TrackedQuestIDs.Empty();

	for (const FName& ID : InCompletedQuests)
	{
		CompletedQuests.Add(ID);
	}

	for (const FQuestSaveEntry& Entry : ActiveQuestSaves)
	{
		if (Entry.QuestID.IsNone()) continue;
		FQuestProgress Progress;
		Progress.Counts = Entry.Counts;
		ActiveQuestProgress.Add(Entry.QuestID, Progress);
	}

	TrackedQuestIDs = InTrackedQuests;

	// 퀘스트 추적 UI 갱신
	OnQuestTrackingChanged.Broadcast();
}

void UOHSMQuestComponent::UntrackQuest(FName QuestID)
{
	if (TrackedQuestIDs.Remove(QuestID) > 0)
	{
		OnQuestTrackingChanged.Broadcast();
	}
}


