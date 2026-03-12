
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