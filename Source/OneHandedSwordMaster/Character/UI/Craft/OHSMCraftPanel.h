// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "OHSMCraftPanel.generated.h"

class UTreeView;
class UOHSMCraftDetailInfo;

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMCraftPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
	UFUNCTION()
	void OnCraftItemClicked(UObject* InItem);

	UFUNCTION()
	void OnCraftItemExpansionChanged(UObject* InItem, bool bIsExpanded);

	UFUNCTION()
	void OnCraftItemGetChildren(UObject* InItem, TArray<UObject*>& OutChildren);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTreeView> CraftItemView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOHSMCraftDetailInfo> CraftDetailInfo;

	/** 제작 레시피 DataTable — BP 또는 PlayerController에서 할당 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craft")
	TObjectPtr<UDataTable> CraftDataTable;
};
