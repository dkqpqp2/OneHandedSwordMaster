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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Default")
	TArray<FName> DefaultEquippedItemIDs;

	void InitializeDefaultEquipment();

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItem(FName ItemID);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItemFromSlot(int32 InvSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipItem(EEquipmentSlot Slot);
	
	UFUNCTION(BlueprintPure, Category = "Equipment")
	FName GetEquippedItem(EEquipmentSlot Slot) const;
	
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsSlotEmpty(EEquipmentSlot Slot) const;
	
	const TMap<EEquipmentSlot, FName>& GetAllEquippedItems() const { return EquippedItems; }

	// 저장 데이터로 장비 복원
	void RestoreFromSave(const TMap<EEquipmentSlot, FName>& SavedEquipment);
	
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

private:
	UPROPERTY()
	TMap<EEquipmentSlot, FName> EquippedItems;
	
	const FItemData* FindItemData(FName ItemID) const;
	
	void ModifyStatBonuses(const FItemData& Data, float Multiplier);
	
	void UpdateVisualMesh(EEquipmentSlot Slot, const FItemData* Data);
};
