#include "OHSMRespawnZone.h"

#include "Components/BoxComponent.h"

AOHSMRespawnZone::AOHSMRespawnZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	RootComponent = ZoneBox;
	ZoneBox->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
	ZoneBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FVector AOHSMRespawnZone::GetRespawnLocation() const
{
	const FVector Extent = ZoneBox->GetScaledBoxExtent();

	const FVector RandomOffset = FVector(
		FMath::RandRange(-Extent.X * 0.8f, Extent.X * 0.8f),
		FMath::RandRange(-Extent.Y * 0.8f, Extent.Y * 0.8f),
		0.0f
	);

	return GetActorLocation() + RandomOffset;
}
