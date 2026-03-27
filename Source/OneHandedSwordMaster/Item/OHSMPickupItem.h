// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMPickupItem.generated.h"

UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMPickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOHSMPickupItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> CollisionSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> ItemMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class URotatingMovementComponent> RotatingMovement;
	
public:
	UPROPERTY(EditAnywhere, BlueprintREadWrite, Category = "Item")
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintREadWrite, Category = "Item")
	int32 ItemCount = 1;
	
	UPROPERTY(EditAnywhere, BlueprintREadWrite, Category = "Item")
	TObjectPtr<class UDataTable> ItemDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float FloatingHeight = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float FloatingSpeed = 2.0f;
	
	UPROPERTY()
	float InitialZ = 0.0f;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeItem(FName InItemID, int32 InCount, UDataTable* InItemDataTable);
	
	const FItemData* GetItemData() const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FName GetItemID() const { return ItemID; }
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	int32 GetItemCount() const { return ItemCount; }
	
protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
