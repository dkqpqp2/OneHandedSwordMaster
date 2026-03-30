// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OHSMQuickSlotComponent.generated.h"

// 슬롯 인덱스, 할당된 아이템 ID
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickSlotUpdated, int32, SlotIndex, FName, ItemID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONEHANDEDSWORDMASTER_API UOHSMQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 0~3: 포션 슬롯, 4~7: 스킬 슬롯
	static constexpr int32 PotionSlotCount = 4;
	static constexpr int32 SkillSlotCount  = 4;
	static constexpr int32 TotalSlotCount  = 8;

	UOHSMQuickSlotComponent();

public:
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot")
	FOnQuickSlotUpdated OnQuickSlotUpdated;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	TArray<FName> QuickSlots;

public:
	// SlotIndex: 0~7
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void AssignItem(int32 SlotIndex, FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void ClearSlot(int32 SlotIndex);

	// PotionSlotIndex: 0~3, 포션 1개 사용 (인벤토리에서 제거)
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void UsePotionSlot(int32 PotionSlotIndex);

	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	FName GetSlotItemID(int32 SlotIndex) const;

	/** 두 퀵슬롯 위치 교체 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SwapSlots(int32 FromSlot, int32 ToSlot);

	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	bool IsSlotEmpty(int32 SlotIndex) const;
};
