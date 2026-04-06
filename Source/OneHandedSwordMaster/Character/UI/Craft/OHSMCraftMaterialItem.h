// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMCraftMaterialItem.generated.h"

struct FCraftMaterialData;
class UImage;
class UTextBlock;

struct FCraftMaterialData;

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMCraftMaterialItem : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMaterialData(const FCraftMaterialData& InMaterialData);

	/** 인벤토리 변경 시 보유 수량만 재갱신 */
	void RefreshCount();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Label_ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Label_Count;

private:
	/** SetMaterialData에서 캐시해둔 데이터 */
	FName   CachedItemID;
	int32   CachedRequiredCount = 0;
};
