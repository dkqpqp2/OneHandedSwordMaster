#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"
#include "OHSMQuestComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted,         FName /*QuestID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestProgressUpdated,  FName /*QuestID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted,        FName /*QuestID*/);
DECLARE_MULTICAST_DELEGATE(FOnQuestTrackingChanged);

/** 퀘스트 진행도 래퍼 (TMap 중첩 컨테이너 UPROPERTY 지원) */
USTRUCT()
struct FQuestProgress
{
	GENERATED_BODY()

	/** Objectives 배열 인덱스에 대응하는 현재 진행 횟수 */
	UPROPERTY()
	TArray<int32> Counts;
};

/**
 * 플레이어 퀘스트 상태를 관리하는 컴포넌트.
 * - QuestDataTable (DT_QuestData) 에서 퀘스트 정의를 읽는다.
 * - AcceptQuest / CompleteQuest / Notify* 메서드로 상태를 갱신한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONEHANDEDSWORDMASTER_API UOHSMQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOHSMQuestComponent();

protected:
	virtual void BeginPlay() override;

protected:
	/** BP_PlayerCharacter 디폴트에서 DT_QuestData 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	TObjectPtr<UDataTable> QuestDataTable;

	// ─── 런타임 상태 ──────────────────────────────────────────────────────

	/** 진행 중인 퀘스트 ID → 목표별 카운트 */
	UPROPERTY()
	TMap<FName, FQuestProgress> ActiveQuestProgress;

	/** 완료된 퀘스트 ID 집합 */
	UPROPERTY()
	TSet<FName> CompletedQuests;

	/** 추적 중인 퀘스트 (순서 유지, 최대 3개) */
	UPROPERTY()
	TArray<FName> TrackedQuestIDs;

public:
	// ─── 퀘스트 수락 / 완료 ──────────────────────────────────────────────

	/** 수락 조건 확인 (선행 퀘스트 완료 + 중복 방지) */
	bool CanAcceptQuest(FName QuestID) const;

	/** 퀘스트 수락 — 성공 시 true */
	bool AcceptQuest(FName QuestID);

	/** 모든 목표가 달성됐는지 */
	bool CanCompleteQuest(FName QuestID) const;

	/** 퀘스트 완료 — 보상 지급 후 ActiveQuest 에서 제거 */
	void CompleteQuest(FName QuestID);

	// ─── 진행도 업데이트 ─────────────────────────────────────────────────

	/** 적 처치 시 호출 (EnemyID 와 Kill 목표 TargetID 비교) */
	void NotifyEnemyKilled(FName EnemyID);

	/** 아이템 획득 시 호출 */
	void NotifyItemCollected(FName ItemID, int32 Count = 1);

	/** NPC 대화 시 호출 */
	void NotifyTalkedToNPC(FName NPCID);

	/** 스킬 습득 시 호출 */
	void NotifySkillLearned(FName SkillID);

	// ─── 조회 ────────────────────────────────────────────────────────────

	EQuestState GetQuestState(FName QuestID) const;

	/** 특정 목표 현재 진행 횟수 */
	int32 GetObjectiveProgress(FName QuestID, int32 ObjectiveIndex) const;

	const FOHSMQuestData* GetQuestData(FName QuestID) const;

	/** 현재 진행 중인 퀘스트 ID 목록 */
	TArray<FName> GetActiveQuestIDs() const;

	bool IsQuestCompleted(FName QuestID) const { return CompletedQuests.Contains(QuestID); }
	bool IsQuestActive(FName QuestID) const    { return ActiveQuestProgress.Contains(QuestID); }

	// ─── 추적 ─────────────────────────────────────────────────────────────

	void TrackQuest(FName QuestID);
	void UntrackQuest(FName QuestID);
	bool IsQuestTracked(FName QuestID) const { return TrackedQuestIDs.Contains(QuestID); }
	const TArray<FName>& GetTrackedQuestIDs() const { return TrackedQuestIDs; }

	UDataTable* GetQuestDataTable() const { return QuestDataTable; }

	// ─── 델리게이트 ──────────────────────────────────────────────────────

	FOnQuestAccepted        OnQuestAccepted;
	FOnQuestProgressUpdated OnQuestProgressUpdated;
	FOnQuestCompleted       OnQuestCompleted;
	FOnQuestTrackingChanged OnQuestTrackingChanged;

private:
	/** 공통 진행도 갱신 */
	void UpdateProgress(EQuestObjectiveType Type, FName TargetID, int32 Count);

	/** 보상 지급 */
	void GiveRewards(const FQuestRewardData& Reward);
};
