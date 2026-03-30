// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMEquipmentComponent.h"

#include "OHSMInventoryComponent.h"
#include "OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Data/OHSMPlayerStatData.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Weapon/OHSMWeaponBase.h"
#include "Components/StaticMeshComponent.h"

UOHSMEquipmentComponent::UOHSMEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOHSMEquipmentComponent::InitializeDefaultEquipment()
{
	for (const FName& ItemID : DefaultEquippedItemIDs)
	{
		if (!ItemID.IsNone())
		{
			EquipItem(ItemID);
		}
	}
}

// ============================================================
// 장착
// ============================================================
bool UOHSMEquipmentComponent::EquipItem(FName ItemID)
{
	if (ItemID.IsNone())
	{
		return false;
	}

	const FItemData* Data = FindItemData(ItemID);
	if (!Data || Data->ItemType != EItemType::Equipment || Data->EquipSlot == EEquipmentSlot::None)
	{
		return false;
	}

	EEquipmentSlot TargetSlot = Data->EquipSlot;

	// 같은 슬롯에 이미 장착된 아이템이 있으면 먼저 해제 (인벤토리로 반환)
	if (!IsSlotEmpty(TargetSlot))
	{
		UnequipItem(TargetSlot);
	}

	// 인벤토리에서 제거 (1개)
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (!Player)
	{
		return false;
	}
	UOHSMInventoryComponent* Inventory = Player->GetInventoryComponent();
	if (!Inventory)
	{
		return false;
	}
	int32 Removed = Inventory->RemoveItem(ItemID, 1);
	if (Removed <= 0)
	{
		return false;
	}

	// 장착 등록
	EquippedItems.Add(TargetSlot, ItemID);

	// 스탯 보너스 적용
	ModifyStatBonuses(*Data, 1.0f);

	// 시각적 메시 부착 (무기/투구만)
	UpdateVisualMesh(TargetSlot, Data);

	OnEquipmentChanged.Broadcast(TargetSlot);
	return true;
}

// ============================================================
// 해제
// ============================================================
void UOHSMEquipmentComponent::UnequipItem(EEquipmentSlot Slot)
{
	FName* EquippedID = EquippedItems.Find(Slot);
	if (!EquippedID || EquippedID->IsNone())
	{
		return;
	}

	FName ItemID = *EquippedID;
	const FItemData* Data = FindItemData(ItemID);

	// 스탯 보너스 제거
	if (Data)
	{
		ModifyStatBonuses(*Data, -1.0f);
	}

	// 시각적 메시 제거
	UpdateVisualMesh(Slot, nullptr);

	// 인벤토리로 반환
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (Player)
	{
		UOHSMInventoryComponent* Inventory = Player->GetInventoryComponent();
		if (Inventory)
		{
			Inventory->AddItem(ItemID, 1, true);  // 장비 해제는 픽업 알림 없이 조용히 추가
		}
	}

	// 슬롯 제거
	EquippedItems.Remove(Slot);

	OnEquipmentChanged.Broadcast(Slot);
}

// ============================================================
// 조회
// ============================================================
FName UOHSMEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	const FName* Found = EquippedItems.Find(Slot);
	return Found ? *Found : NAME_None;
}

bool UOHSMEquipmentComponent::IsSlotEmpty(EEquipmentSlot Slot) const
{
	const FName* Found = EquippedItems.Find(Slot);
	return !Found || Found->IsNone();
}

// ============================================================
// 내부: 아이템 데이터 조회
// ============================================================
const FItemData* UOHSMEquipmentComponent::FindItemData(FName ItemID) const
{
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (!Player)
	{
		return nullptr;
	}
	UOHSMInventoryComponent* Inventory = Player->GetInventoryComponent();
	if (!Inventory)
	{
		return nullptr;
	}
	return Inventory->GetItemData(ItemID);
}

// ============================================================
// 내부: 스탯 보너스 적용/제거
// ============================================================
void UOHSMEquipmentComponent::ModifyStatBonuses(const FItemData& Data, float Multiplier)
{
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (!Player)
	{
		return;
	}
	UOHSMPlayerStatComponent* StatComp = Player->GetStatComponent();
	if (!StatComp)
	{
		return;
	}

	if (Data.BonusAtk > 0.0f)
	{
		StatComp->AddEquipmentBonus(EPlayerStatType::Damage, Data.BonusAtk * Multiplier);
	}
	if (Data.BonusDef > 0.0f)
	{
		StatComp->AddEquipmentBonus(EPlayerStatType::Defense, Data.BonusDef * Multiplier);
	}
	if (Data.BonusMaxHp > 0.0f)
	{
		StatComp->AddEquipmentBonus(EPlayerStatType::MaxHealth, Data.BonusMaxHp * Multiplier);
	}
	if (Data.BonusMaxMp > 0.0f)
	{
		StatComp->AddEquipmentBonus(EPlayerStatType::MaxMana, Data.BonusMaxMp * Multiplier);
	}
}

// ============================================================
// 내부: 시각적 메시 업데이트 (무기 / 투구)
// ============================================================
void UOHSMEquipmentComponent::UpdateVisualMesh(EEquipmentSlot Slot, const FItemData* Data)
{
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (!Player)
	{
		return;
	}

	if (Slot == EEquipmentSlot::Weapon)
	{
		// CurrentWeapon 액터의 WeaponMesh에 메시 적용
		AOHSMWeaponBase* CurrentWeapon = Player->GetCurrentWeapon();
		if (CurrentWeapon)
		{
			UStaticMeshComponent* WeaponMesh = CurrentWeapon->GetWeaponMesh();
			if (WeaponMesh)
			{
				WeaponMesh->SetStaticMesh(Data ? Data->ItemMesh : nullptr);
			}
		}
	}
	// 투구는 스탯만 적용, 비주얼 없음
}
