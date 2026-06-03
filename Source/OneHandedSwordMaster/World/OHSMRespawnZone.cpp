#include "OHSMRespawnZone.h"

#include "Components/BoxComponent.h"

// 리스폰 존 초기화
AOHSMRespawnZone::AOHSMRespawnZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	RootComponent = ZoneBox;
	ZoneBox->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f)); // 기본 크기
	ZoneBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 없음
}

// 리스폰 위치 반환
FVector AOHSMRespawnZone::GetRespawnLocation() const
{
	const FVector Extent = ZoneBox->GetScaledBoxExtent(); // 박스 크기

	const FVector RandomOffset = FVector(
		FMath::RandRange(-Extent.X * 0.8f, Extent.X * 0.8f), // X 랜덤 오프셋
		FMath::RandRange(-Extent.Y * 0.8f, Extent.Y * 0.8f), // Y 랜덤 오프셋
		0.0f
	);

	return GetActorLocation() + RandomOffset; // 최종 위치 반환
}
