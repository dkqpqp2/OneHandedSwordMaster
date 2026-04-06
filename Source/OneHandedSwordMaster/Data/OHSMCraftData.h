// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OHSMCraftData.generated.h"

/** 제작에 필요한 재료 1종 */
USTRUCT(BlueprintType)
struct FCraftMaterialData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craft")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craft", meta = (ClampMin = "1"))
	int32 RequiredCount = 1;

	FCraftMaterialData() : ItemID(NAME_None), RequiredCount(1) {}
};

/** 제작 레시피 1행 — DT_CraftItemData DataTable 행 구조 */
USTRUCT(BlueprintType)
struct FOHSMCraftItemData : public FTableRowBase
{
	GENERATED_BODY()

	/** 결과 아이템 ID (DT_ItemData Row Name과 동일) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craft")
	FName ResultItemID;

	/** 결과 아이템 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craft", meta = (ClampMin = "1"))
	int32 ResultCount = 1;

	/** 필요 재료 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craft")
	TArray<FCraftMaterialData> Materials;

	/** 제작 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craft")
	FText Description;

	FOHSMCraftItemData() : ResultItemID(NAME_None), ResultCount(1) {}
};
