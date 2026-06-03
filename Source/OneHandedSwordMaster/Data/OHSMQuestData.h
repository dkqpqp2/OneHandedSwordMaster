#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OHSMQuestData.generated.h"

// ─── 목표 유형 ──────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuestObjectiveType : uint8
{
	Kill		UMETA(DisplayName = "처치"),
	Collect		UMETA(DisplayName = "수집"),
	Talk		UMETA(DisplayName = "대화"),
	LearnSkill	UMETA(DisplayName = "스킬 습득")
};

// ─── 퀘스트 상태 ────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	NotStarted	UMETA(DisplayName = "시작 전"),
	Active		UMETA(DisplayName = "진행 중"),
	Completable	UMETA(DisplayName = "완료 가능"),
	Completed	UMETA(DisplayName = "완료됨")
};

// ─── 단일 목표 ──────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuestObjectiveData
{
	GENERATED_BODY()

	/** 목표 유형 (처치 / 수집 / 대화) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestObjectiveType ObjectiveType = EQuestObjectiveType::Kill;

	/** 처치: EnemyID / 수집: ItemID / 대화: NPCID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName TargetID;

	/** 목표 설명 텍스트 (예: "골렘 5마리 처치") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	/** 필요 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 RequiredCount = 1;
};

// ─── 보상 ────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuestRewardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward", meta = (ClampMin = "0"))
	int32 RewardExp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward", meta = (ClampMin = "0"))
	int32 RewardSkillPoints = 0;

	/** 보상 아이템 ID 목록 (DT_ItemData Row Name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	TArray<FName> RewardItemIDs;

	/** RewardItemIDs 와 1:1 대응하는 수량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	TArray<int32> RewardItemCounts;
};

// ─── 퀘스트 데이터 (DataTable Row) ──────────────────────────────────────────

USTRUCT(BlueprintType)
struct FOHSMQuestData : public FTableRowBase
{
	GENERATED_BODY()

	/** 퀘스트 고유 ID — DataTable Row Name 과 동일하게 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	/** 선행 퀘스트 ID 목록 — 모두 완료돼야 수락 가능 (없으면 빈 배열) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FName> PrerequisiteQuestIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestObjectiveData> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FQuestRewardData Reward;
};

// ─── 맵 이동 시 퀘스트 진행도 저장용 ────────────────────────────────────────

USTRUCT()
struct FQuestSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName QuestID;

	/** Objectives 배열 순서에 대응하는 현재 카운트 */
	UPROPERTY()
	TArray<int32> Counts;
};
