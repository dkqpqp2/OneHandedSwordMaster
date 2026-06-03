// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMCraftInfo.generated.h"

struct FOHSMCraftItemData;
class UImage;
class UTextBlock;
class UWrapBox;
class UOHSMCraftMaterialItem;
class UPanelSlot;
class UOHSMInventoryComponent;

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMCraftInfo : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetCraftItemData(const FOHSMCraftItemData& InData, FName InRecipeID);

	/** 인벤토리 변경 시 재료 위젯의 보유 수량만 재갱신 */
	void RefreshMaterialCounts();

	/** 위젯 재사용 시 내용 초기화 */
	void ResetInfo();

protected:
	virtual void NativeDestruct() override;

	/**
	 * 인벤토리 변경 콜백 — SetCraftItemData에서 자동 구독.
	 * UOHSMCraftDetailInfo 등 파생 클래스에서 override해 추가 처리 가능.
	 */
	UFUNCTION()
	virtual void OnInventoryUpdated(int32 SlotIndex);

	// 결과 아이템
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ResultItem;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Label_ResultName;

	// 재료 목록 (가로 나열 + 자동 줄바꿈)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> WrapBox_Material;

	UPROPERTY(EditAnywhere, Category = "Craft")
	TSubclassOf<UOHSMCraftMaterialItem> MaterialWidgetClass;

private:
	/** OnInventoryUpdated 구독 해제·재구독용 캐시 */
	UPROPERTY()
	TObjectPtr<UOHSMInventoryComponent> CachedInventoryComp;
};
