#include "OHSMQuestObjectiveMarker.h"

#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"

AOHSMQuestObjectiveMarker::AOHSMQuestObjectiveMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	RootComponent = Billboard;

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MarkerMesh->SetupAttachment(RootComponent);
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerMesh->SetCastShadow(false);
}
