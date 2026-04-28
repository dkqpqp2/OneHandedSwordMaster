#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OHSMRespawnZone.generated.h"

/**
 * 레벨에 배치하는 리스폰 구역.
 * 우선순위가 낮을수록 먼저 선택됩니다.
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMRespawnZone : public AActor
{
	GENERATED_BODY()

public:
	AOHSMRespawnZone();

	/** 구역 내 랜덤 리스폰 위치 반환 */
	FVector GetRespawnLocation() const;

	int32 GetPriority() const { return Priority; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone")
	TObjectPtr<class UBoxComponent> ZoneBox;

	/** 여러 구역이 있을 때 우선순위 — 낮을수록 먼저 선택 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	int32 Priority = 0;
};
