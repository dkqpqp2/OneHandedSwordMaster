// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMEquipmentComponent.generated.h"

// 장비 슬롯 변경 시 브로드캐스트 (어떤 슬롯이 바뀌었는지)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentChanged, EEquipmentSlot, ChangedSlot);

/**
 * 플레이어 장비 관리 컴포넌트
 * - 5개 슬롯 (무기/투구/갑옷/바지/신발) 관리
 * - 장착 시 스탯 보너스 자동 적용 / 해제 시 제거
 * - 무기·투구는 시각적 메시 부착
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONEHANDEDSWORDMASTER_API UOHSMEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOHSMEquipmentComponent();

	/** 게임 시작 시 기본으로 장착할 아이템 ID 목록 (Blueprint에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Default")
	TArray<FName> DefaultEquippedItemIDs;

	/** 기본 장착 아이템 적용 - PlayerCharacter::BeginPlay에서 CurrentWeapon 스폰 이후 호출 */
	void InitializeDefaultEquipment();

public:
	/**
	 * 아이템 장착
	 * - ItemData의 EquipSlot을 읽어 해당 슬롯에 장착
	 * - 같은 슬롯에 이미 장착된 아이템이 있으면 먼저 해제 후 인벤토리로 반환
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItem(FName ItemID);

	/** 인벤토리의 특정 슬롯 인덱스로부터 장착 (우클릭 장착 시 사용) */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItemFromSlot(int32 InvSlotIndex);

	/**
	 * 특정 슬롯 해제 → 아이템을 인벤토리로 반환
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipItem(EEquipmentSlot Slot);

	/** 슬롯에 장착된 아이템 ID (없으면 NAME_None) */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	FName GetEquippedItem(EEquipmentSlot Slot) const;

	/** 슬롯이 비어있는지 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsSlotEmpty(EEquipmentSlot Slot) const;

	/** 전체 장착 맵 반환 (UI 초기화에 사용) */
	const TMap<EEquipmentSlot, FName>& GetAllEquippedItems() const { return EquippedItems; }

	/** 장비 슬롯 변경 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

private:
	// 현재 장착 중인 아이템 (슬롯 → ItemID)
	UPROPERTY()
	TMap<EEquipmentSlot, FName> EquippedItems;

	// 아이템 데이터 조회
	const FItemData* FindItemData(FName ItemID) const;

	// 스탯 보너스 적용 (Multiplier: +1 장착, -1 해제)
	void ModifyStatBonuses(const FItemData& Data, float Multiplier);

	// 시각적 메시 업데이트 (nullptr = 제거)
	void UpdateVisualMesh(EEquipmentSlot Slot, const FItemData* Data);
};
