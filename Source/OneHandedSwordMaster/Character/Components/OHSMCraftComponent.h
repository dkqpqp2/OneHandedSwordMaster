// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OHSMCraftComponent.generated.h"

struct FOHSMCraftItemData;

/** 제작 완료 알림 (RecipeID, 결과 아이템 ID, 결과 개수) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCraftCompleted, FName, RecipeID, FName, ResultItemID, int32, ResultCount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONEHANDEDSWORDMASTER_API UOHSMCraftComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOHSMCraftComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 제작 성공 시 브로드캐스트 */
	UPROPERTY(BlueprintAssignable, Category = "Craft")
	FOnCraftCompleted OnCraftCompleted;

	/** 레시피 ID로 아이템 제작 — 재료 부족 시 false 반환 */
	UFUNCTION(BlueprintCallable, Category = "Craft")
	bool CraftItem(FName RecipeID);

	/** 제작 가능 여부 확인 (재료 보유 체크) */
	UFUNCTION(BlueprintPure, Category = "Craft")
	bool CanCraftItem(FName RecipeID) const;

	UDataTable* GetCraftDataTable() const { return CraftDataTable; }

	/** 제작 레시피 DataTable (BP에서 할당) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UDataTable> CraftDataTable;

private:
	const FOHSMCraftItemData* FindRecipe(FName RecipeID) const;

	UPROPERTY()
	TObjectPtr<class UOHSMInventoryComponent> InventoryComp;
};
