#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OHSMQuestObjectiveMarker.generated.h"

/**
 * 레벨에 배치하는 퀘스트 목표 위치 마커.
 *
 * Talk 목표는 AOHSMQuestNPC 위치를 자동으로 사용하므로 마커 불필요.
 * Kill / Collect / LearnSkill 목표는 이 액터를 레벨에 배치해 목적지를 지정한다.
 *
 * ── 에디터 설정 ──────────────────────────────────────────────
 *  QuestID        : 연결할 퀘스트 ID (DataTable Row Name)
 *  ObjectiveIndex : 몇 번째 목표인지 (0-based)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMQuestObjectiveMarker : public AActor
{
	GENERATED_BODY()

public:
	AOHSMQuestObjectiveMarker();

	/** 이 마커가 연결된 퀘스트 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName QuestID;

	/** 이 마커가 연결된 목표 인덱스 (0-based) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	int32 ObjectiveIndex = 0;

protected:
	/** 에디터에서 위치 확인용 (런타임에는 숨겨짐) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UBillboardComponent> Billboard;

	/** 런타임 비주얼 메시 (BP에서 원하는 메시/이펙트로 교체 가능) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MarkerMesh;
};
