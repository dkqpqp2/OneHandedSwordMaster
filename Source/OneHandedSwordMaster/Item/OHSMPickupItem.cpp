// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPickupItem.h"

#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"

// Sets default values
AOHSMPickupItem::AOHSMPickupItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	
	CollisionSphere->SetSphereRadius(80.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
    
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemMesh->SetCastShadow(true);
	
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
	
	

}

// Called when the game starts or when spawned
void AOHSMPickupItem::BeginPlay()
{
	Super::BeginPlay();
	
	if (!CollisionSphere)
	{
		return;
	}

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AOHSMPickupItem::OnOverlapBegin);
	
	
	InitialZ = GetActorLocation().Z;
}

// Called every frame
void AOHSMPickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Floating 효과
	if (FloatingSpeed > 0.0f && FloatingHeight > 0.0f)
	{
		float NewZ = FMath::Sin(GetWorld()->GetTimeSeconds() * FloatingSpeed) * FloatingHeight;
		FVector NewLocation = GetActorLocation();
		NewLocation.Z = GetActorLocation().Z + NewZ * DeltaTime;
		SetActorLocation(NewLocation);
	}
}

void AOHSMPickupItem::InitializeItem(FName InItemID, int32 InCount, UDataTable* InItemDataTable)
{
	ItemID = InItemID;
	ItemCount = InCount;
	ItemDataTable = InItemDataTable;
}

const FItemData* AOHSMPickupItem::GetItemData() const
{
	if (!ItemDataTable || ItemID.IsNone())
	{
		return nullptr;
	}
    
	return ItemDataTable->FindRow<FItemData>(ItemID, TEXT("GetItemData"));
}

void AOHSMPickupItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }
	
    AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }
    // 인벤토리 확인
    UOHSMInventoryComponent* InventoryComp = Player->GetInventoryComponent();
    if (!InventoryComp)
    {
        return;
    }
	
    int32 AddedCount = InventoryComp->AddItem(ItemID, ItemCount);
	
    if (AddedCount > 0)
    {
        Destroy();
    }

}

