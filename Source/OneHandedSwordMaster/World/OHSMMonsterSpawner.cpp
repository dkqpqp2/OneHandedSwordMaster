#include "OHSMMonsterSpawner.h"

#include "NavigationSystem.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

AOHSMMonsterSpawner::AOHSMMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

// ─── 생명주기 ─────────────────────────────────────────────────────────────────

void AOHSMMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSpawn)
	{
		SpawnAllMonsters();
	}
}

// ─── 스폰 ─────────────────────────────────────────────────────────────────────

void AOHSMMonsterSpawner::SpawnAllMonsters()
{
	if (!MonsterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] %s: MonsterClass 가 설정되지 않았습니다."), *GetName());
		return;
	}

	for (int32 i = 0; i < MaxSpawnCount; ++i)
	{
		SpawnOneMonster();
	}
}

void AOHSMMonsterSpawner::SpawnOneMonster()
{
	if (!MonsterClass)
	{
		return;
	}

	// NavMesh 위 랜덤 위치 탐색
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return;
	}

	FNavLocation RandomLocation;
	const bool bFound = NavSys->GetRandomReachablePointInRadius(
		GetActorLocation(), SpawnRadius, RandomLocation);

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] %s: NavMesh 위 스폰 위치를 찾지 못했습니다."), *GetName());
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AOHSMEnemyBase* Monster = GetWorld()->SpawnActor<AOHSMEnemyBase>(
		MonsterClass,
		RandomLocation.Location,
		FRotator::ZeroRotator,
		Params);

	if (!Monster)
	{
		return;
	}

	// 사망 델리게이트 구독
	Monster->OnEnemyDeathNotify.AddUObject(this, &AOHSMMonsterSpawner::OnMonsterDied);
	SpawnedMonsters.Add(Monster);
}

// ─── 사망 / 리스폰 ───────────────────────────────────────────────────────────

void AOHSMMonsterSpawner::OnMonsterDied(AOHSMEnemyBase* DeadMonster)
{
	SpawnedMonsters.Remove(DeadMonster);

	if (RespawnDelay <= 0.f)
	{
		return;
	}

	// 리스폰 타이머 — 각 사망마다 독립 타이머로 슬롯 보충
	PendingRespawnCount++;

	FTimerHandle RespawnHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RespawnHandle,
		this,
		&AOHSMMonsterSpawner::RespawnOne,
		RespawnDelay,
		/*bLoop=*/false);
}

void AOHSMMonsterSpawner::RespawnOne()
{
	PendingRespawnCount = FMath::Max(0, PendingRespawnCount - 1);
	SpawnOneMonster();
}

// ─── 조회 ────────────────────────────────────────────────────────────────────

int32 AOHSMMonsterSpawner::GetAliveCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<AOHSMEnemyBase>& Monster : SpawnedMonsters)
	{
		if (IsValid(Monster) && !Monster->IsDead())
		{
			Count++;
		}
	}
	return Count;
}
