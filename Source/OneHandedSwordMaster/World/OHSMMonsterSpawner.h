#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OHSMMonsterSpawner.generated.h"

class AOHSMEnemyBase;

/**
 * 레벨에 배치하여 몬스터를 관리하는 스포너.
 *
 * ── 에디터 설정 항목 ──────────────────────────────────────────
 *  MonsterClass    : 스폰할 몬스터 BP 클래스
 *  MaxSpawnCount   : 동시 유지할 최대 몬스터 수
 *  SpawnRadius     : 스포너 중심 반경 (NavMesh 위 랜덤 위치)
 *  RespawnDelay    : 사망 후 리스폰까지 대기 시간 (0 = 리스폰 없음)
 *  bAutoSpawn      : BeginPlay 에서 자동 스폰 여부
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	AOHSMMonsterSpawner();

protected:
	virtual void BeginPlay() override;

	// ── 스포너 설정 ───────────────────────────────────────────────────────────

	/** 스폰할 몬스터 클래스 (BP_EnemyWhite 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AOHSMEnemyBase> MonsterClass;

	/** 동시에 유지할 최대 몬스터 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxSpawnCount = 3;

	/** 스포너 중심 기준 스폰 반경 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "100"))
	float SpawnRadius = 400.f;

	/** 몬스터 사망 후 리스폰 대기 시간 (초). 0 이하이면 리스폰 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0"))
	float RespawnDelay = 15.f;

	/** true 이면 BeginPlay 에서 MaxSpawnCount 만큼 즉시 스폰 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bAutoSpawn = true;

public:
	/** 외부에서 강제로 몬스터를 모두 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnAllMonsters();

	/** 현재 살아있는 몬스터 수 반환 */
	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetAliveCount() const;

private:
	/** 현재 스폰된 몬스터 목록 */
	UPROPERTY()
	TArray<TObjectPtr<AOHSMEnemyBase>> SpawnedMonsters;

	/** 리스폰 대기 중인 슬롯 수 */
	int32 PendingRespawnCount = 0;

	/** 몬스터 1마리를 NavMesh 위 랜덤 위치에 스폰 */
	void SpawnOneMonster();

	/** 몬스터 사망 시 호출 */
	void OnMonsterDied(AOHSMEnemyBase* DeadMonster);

	/** RespawnDelay 후 호출 — 슬롯 1개 보충 */
	void RespawnOne();
};
