// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMCraftInfo.h"
#include "OHSMCraftDetailInfo.generated.h"

class UButton;
class UTextBlock;
class UOHSMCraftComponent;
class UOHSMInventoryComponent;

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMCraftDetailInfo : public UOHSMCraftInfo
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SetCraftItemData(const FOHSMCraftItemData& InData, FName InRecipeID) override;

protected:
	UFUNCTION()
	void OnClickedCraft();

	/** 인벤토리 변경 시 제작 버튼 상태 갱신 (UOHSMCraftInfo::OnInventoryUpdated override) */
	virtual void OnInventoryUpdated(int32 SlotIndex) override;

	void UpdateCraftButton();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Label_Description;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Craft;

	UPROPERTY()
	TObjectPtr<UOHSMCraftComponent> CraftComp;

	UPROPERTY()
	TObjectPtr<UOHSMInventoryComponent> InventoryComp;

	FName CurrentRecipeID;
};
