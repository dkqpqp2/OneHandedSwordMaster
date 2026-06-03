#include "OHSMMonsterSpawner.h"

#include "NavigationSystem.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

AOHSMMonsterSpawner::AOHSMMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	// 에디터에서 위치를 확인할 수 있도록 빌보드 아이콘 추가 (게임에서는 안 보임)
	UBillboardComponent* Billboard = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (Billboard)
	{
		RootComponent = Billboard;
	}
#endif
}

// ─── 생명주기 ─────────────────────────────────────────────────────────────────

// 게임 시작 처리
void AOHSMMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSpawn)
	{
		SpawnAllMonsters(); // 자동 스폰
	}
}

// ─── 스폰 ─────────────────────────────────────────────────────────────────────

// 전체 몬스터 스폰
void AOHSMMonsterSpawner::SpawnAllMonsters()
{
	UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] SpawnAllMonsters 호출 — MonsterClass:%s MaxCount:%d"),
		MonsterClass ? *MonsterClass->GetName() : TEXT("NULL"), MaxSpawnCount);

	if (!MonsterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[MonsterSpawner] %s: MonsterClass 가 설정되지 않았습니다!"), *GetName());
		return;
	}

	for (int32 i = 0; i < MaxSpawnCount; ++i)
	{
		SpawnOneMonster(); // 개별 스폰
	}
}

// 몬스터 한 마리 스폰
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
		GetActorLocation(), SpawnRadius, RandomLocation); // 랜덤 위치 탐색

	UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] NavMesh 위치 탐색 — SpawnerLoc:%s Radius:%.0f Found:%d"),
		*GetActorLocation().ToString(), SpawnRadius, bFound ? 1 : 0);

	if (!bFound)
	{
		UE_LOG(LogTemp, Error, TEXT("[MonsterSpawner] %s: NavMesh 위 스폰 위치를 찾지 못했습니다! NavMeshBoundsVolume이 스포너를 덮고 있는지 확인하세요."), *GetName());
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// NavMesh 좌표는 바닥 표면 Z값 → 캡슐 중심을 바닥 위에 두려면 반경만큼 올려야 함
	// CDO에서 실제 캡슐 반경 읽어서 정확한 오프셋 계산
	float CapsuleHalfHeight = 96.0f;
	if (ACharacter* CDO = Cast<ACharacter>(MonsterClass->GetDefaultObject()))
	{
		if (UCapsuleComponent* Capsule = CDO->GetCapsuleComponent())
		{
			CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
	}
	const FVector SpawnLocation = RandomLocation.Location + FVector(0.f, 0.f, CapsuleHalfHeight);

	AOHSMEnemyBase* Monster = GetWorld()->SpawnActor<AOHSMEnemyBase>(
		MonsterClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		Params);

	if (!Monster)
	{
		return;
	}

	// 사망 델리게이트 구독
	Monster->OnEnemyDeathNotify.AddUObject(this, &AOHSMMonsterSpawner::OnMonsterDied);
	SpawnedMonsters.Add(Monster); // 목록에 추가
}

// ─── 사망 / 리스폰 ───────────────────────────────────────────────────────────

// 몬스터 사망 처리
void AOHSMMonsterSpawner::OnMonsterDied(AOHSMEnemyBase* DeadMonster)
{
	SpawnedMonsters.Remove(DeadMonster); // 목록에서 제거

	if (RespawnDelay <= 0.f)
	{
		return;
	}

	// 리스폰 타이머 — 각 사망마다 독립 타이머로 슬롯 보충
	PendingRespawnCount++; // 대기 카운트 증가

	FTimerHandle RespawnHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RespawnHandle,
		this,
		&AOHSMMonsterSpawner::RespawnOne,
		RespawnDelay,
		/*bLoop=*/false); // 리스폰 타이머
}

// 몬스터 한 마리 리스폰
void AOHSMMonsterSpawner::RespawnOne()
{
	PendingRespawnCount = FMath::Max(0, PendingRespawnCount - 1); // 카운트 감소
	SpawnOneMonster(); // 재스폰
}

// ─── 조회 ────────────────────────────────────────────────────────────────────

// 생존 몬스터 수 반환
int32 AOHSMMonsterSpawner::GetAliveCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<AOHSMEnemyBase>& Monster : SpawnedMonsters)
	{
		if (IsValid(Monster) && !Monster->IsDead())
		{
			Count++; // 살아있는 몬스터
		}
	}
	return Count;
}
