// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCraftComponent.h"

#include "OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCraftData.h"

UOHSMCraftComponent::UOHSMCraftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOHSMCraftComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryComp = GetOwner()->FindComponentByClass<UOHSMInventoryComponent>();
}

bool UOHSMCraftComponent::CraftItem(FName RecipeID)
{
	if (!CanCraftItem(RecipeID))
	{
		return false;
	}

	const FOHSMCraftItemData* Recipe = FindRecipe(RecipeID);
	if (!Recipe)
	{
		return false;
	}

	// 재료 소모
	for (const FCraftMaterialData& Material : Recipe->Materials)
	{
		InventoryComp->RemoveItem(Material.ItemID, Material.RequiredCount);
	}

	// 결과 아이템 추가
	InventoryComp->AddItem(Recipe->ResultItemID, Recipe->ResultCount);

	// 제작 완료 알림
	OnCraftCompleted.Broadcast(RecipeID, Recipe->ResultItemID, Recipe->ResultCount);

	return true;
}

bool UOHSMCraftComponent::CanCraftItem(FName RecipeID) const
{
	if (!IsValid(InventoryComp))
	{
		return false;
	}

	const FOHSMCraftItemData* Recipe = FindRecipe(RecipeID);
	if (!Recipe)
	{
		return false;
	}

	for (const FCraftMaterialData& Material : Recipe->Materials)
	{
		if (InventoryComp->GetItemCount(Material.ItemID) < Material.RequiredCount)
		{
			return false;
		}
	}

	return true;
}

const FOHSMCraftItemData* UOHSMCraftComponent::FindRecipe(FName RecipeID) const
{
	if (!IsValid(CraftDataTable))
	{
		return nullptr;
	}

	return CraftDataTable->FindRow<FOHSMCraftItemData>(RecipeID, TEXT(""));
}
