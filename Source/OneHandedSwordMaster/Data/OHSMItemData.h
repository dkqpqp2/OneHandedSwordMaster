
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

// 장비 슬롯 종류
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None	UMETA(DisplayName = "없음"),
	Weapon	UMETA(DisplayName = "무기"),
	Helmet	UMETA(DisplayName = "투구"),
	Armor	UMETA(DisplayName = "갑옷"),
	Pants	UMETA(DisplayName = "바지"),
	Shoes	UMETA(DisplayName = "신발")
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
	TObjectPtr<UStaticMesh> ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float MeshScale = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FVector MeshOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackSize = 99;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 SellPrice = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable", meta = (ClampMin = "0"))
	float HealHpAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable", meta = (ClampMin = "0"))
	float HealMpAmount = 0.0f;

	// ─── 장비 전용 ────────────────────────────────────────────
	/** 이 아이템이 들어가는 장비 슬롯 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment")
	EEquipmentSlot EquipSlot = EEquipmentSlot::None;

	/** 공격력 보너스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment", meta = (ClampMin = "0"))
	float BonusAtk = 0.0f;

	/** 방어력 보너스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment", meta = (ClampMin = "0"))
	float BonusDef = 0.0f;

	/** 최대 체력 보너스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment", meta = (ClampMin = "0"))
	float BonusMaxHp = 0.0f;

	/** 최대 마나 보너스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment", meta = (ClampMin = "0"))
	float BonusMaxMp = 0.0f;

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
		SellPrice(0),
		HealHpAmount(0.0f),
		HealMpAmount(0.0f),
		EquipSlot(EEquipmentSlot::None),
		BonusAtk(0.0f),
		BonusDef(0.0f),
		BonusMaxHp(0.0f),
		BonusMaxMp(0.0f)
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