#include "OHSMNavDotActor.h"

#include "Components/InstancedStaticMeshComponent.h"

AOHSMNavDotActor::AOHSMNavDotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DotISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("DotISM"));
	RootComponent = DotISM;
	DotISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DotISM->SetCastShadow(false);
	DotISM->SetReceivesDecals(false);
}
