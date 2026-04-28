#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OHSMQuestNavigationComponent.generated.h"

class UOHSMQuestComponent;
class AOHSMNavDotActor;

/**
 * 현재 추적 중인 퀘스트의 목적지까지 바닥 점선 경로를 표시하는 컴포넌트.
 *
 * ── 동작 방식 ─────────────────────────────────────────────────
 *  Talk 목표    → AOHSMQuestNPC.NPC_ID 로 NPC 위치 자동 탐색
 *  그 외 목표  → 레벨에 배치된 AOHSMQuestObjectiveMarker 위치 사용
 *
 * ── BP에서 설정할 항목 ─────────────────────────────────────────
 *  DotMesh      : 점으로 사용할 StaticMesh (작은 원반 권장)
 *  DotMaterial  : 점 머티리얼 (선택)
 *  DotSpacing   : 점 간격 (기본 150cm)
 *  DotScale     : 점 크기 (기본 0.25 × 0.25 × 0.05)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONEHANDEDSWORDMASTER_API UOHSMQuestNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOHSMQuestNavigationComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * 명시적 네비게이션 대상 퀘스트 설정.
	 * NAME_None 을 전달하면 추적 목록 첫 번째 퀘스트로 자동 복귀.
	 */
	void SetManualNavTarget(FName QuestID);

	/**
	 * 지정 퀘스트의 현재 미완료 목표 위치를 월드에서 탐색 (공개 유틸).
	 * Talk 목표 → NPC 위치, 그 외 → 마커 위치.
	 * @return 위치를 찾으면 true
	 */
	bool FindQuestTargetLocation(FName QuestID, FVector& OutLocation) const;

protected:
	/** 바닥에 표시할 점 메시 (에디터 또는 BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation|Visual")
	TObjectPtr<UStaticMesh> DotMesh;

	/** 점 머티리얼 (선택) */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation|Visual")
	TObjectPtr<UMaterialInterface> DotMaterial;

	/** 점과 점 사이 간격 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float DotSpacing = 150.f;

	/** 점 스케일 (XY=반지름, Z=두께) */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	FVector DotScale = FVector(0.25f, 0.25f, 0.05f);

	/** 경로 재계산 주기 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float UpdateInterval = 0.3f;

	/** 이 거리 이하이면 도착으로 간주, 점선 숨김 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float ArrivalDistance = 300.f;

private:
	UPROPERTY()
	TObjectPtr<UOHSMQuestComponent> QuestComponent;

	UPROPERTY()
	TObjectPtr<AOHSMNavDotActor> NavDotActor;

	FTimerHandle UpdateTimerHandle;

	/** 명시적으로 지정된 네비게이션 대상 퀘스트 (NAME_None 이면 자동 선택) */
	FName ManualNavQuestID = NAME_None;

	// ── 내부 메서드 ────────────────────────────────────────────
	void InitializeNavigation(UOHSMQuestComponent* InQuestComp);

	void OnTrackingChanged();
	void OnQuestProgressUpdated(FName QuestID);

	void StartUpdateTimer();
	void TickUpdate();

	/** 현재 네비게이션 대상 퀘스트의 미완료 목표 위치를 반환 */
	bool FindCurrentTarget(FVector& OutLocation) const;

	/** NavMesh 경로를 따라 점 인스턴스 재배치 */
	void RebuildDots(const FVector& TargetLocation);

	/** 모든 점 제거 */
	void ClearDots();
};
