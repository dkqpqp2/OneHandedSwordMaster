
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OHSMItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Equipment	UMETA(DisplayName = "장비"),
	Consumable	UMETA(DisplayName = "소비"),
	Material	UMETA(DisplayName = "재료"),
	Quest		UMETA(DisplayName = "퀘스트")
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UTexture2D> ItemIcon;

	// 월드에 드롭됐을 때 표시할 3D 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UStaticMesh> ItemMesh;

	// 월드에 드롭됐을 때 메시 크기 (기본값 1.0 = 원본 크기)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float MeshScale = 1.0f;

	// Pivot이 잘못된 메시의 위치 보정값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FVector MeshOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackSize = 99;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 SellPrice = 0;
	
	FItemData() :
		ItemID(NAME_None),
		ItemName(FText::GetEmpty()),
		Description(FText::GetEmpty()),
		ItemType(EItemType::Material),
		ItemIcon(nullptr),
		ItemMesh(nullptr),
		MeshScale(1.0f),
		MeshOffset(FVector::ZeroVector),
		MaxStackSize(99),
		SellPrice(0)
	{}
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Count = 0;
	
	bool IsEmpty() const
	{
		return ItemID.IsNone() || Count <= 0;
	}
	
	void SlotClear()
	{
		ItemID = NAME_None;
		Count = 0;
	}
	
	FInventorySlot() :
		ItemID(NAME_None),
		Count(0)
	{}
	
	FInventorySlot(FName InItemID, int32 InCount) : 
		ItemID(InItemID),
		Count(InCount)
	{}
};

USTRUCT(BlueprintType)
struct FDropItemData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 MinCount = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 MaxCount = 1;
};