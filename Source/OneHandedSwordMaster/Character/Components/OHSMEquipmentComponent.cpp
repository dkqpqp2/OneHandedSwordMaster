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

// 저장 데이터로 장비 복원
void UOHSMEquipmentComponent::RestoreFromSave(const TMap<EEquipmentSlot, FName>& SavedEquipment)
{
	// 기존 장비 초기화 (스탯 제거 없이 직접 비움 — 이전 스탯이 없으므로 안전)
	EquippedItems.Empty();

	for (const auto& Pair : SavedEquipment)
	{
		const EEquipmentSlot Slot = Pair.Key;
		const FName          ItemID = Pair.Value;
		if (ItemID.IsNone()) continue;

		const FItemData* Data = FindItemData(ItemID);
		if (!Data) continue;

		EquippedItems.Add(Slot, ItemID);
		ModifyStatBonuses(*Data, 1.0f);   // 스탯 보너스 재적용
		UpdateVisualMesh(Slot, Data);     // 비주얼 메시 재적용

		OnEquipmentChanged.Broadcast(Slot);
	}
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

// 인벤토리 슬롯 아이템 장착
bool UOHSMEquipmentComponent::EquipItemFromSlot(int32 InvSlotIndex)
{
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

	const FInventorySlot* Slot = Inventory->GetSlot(InvSlotIndex);
	if (!Slot || Slot->IsEmpty())
	{
		return false;
	}

	FName ItemID = Slot->ItemID;

	const FItemData* Data = FindItemData(ItemID);
	if (!Data)
	{
		return false;
	}


	if (Data->ItemType != EItemType::Equipment || Data->EquipSlot == EEquipmentSlot::None)
	{
		return false;
	}

	EEquipmentSlot TargetSlot = Data->EquipSlot;

	if (!IsSlotEmpty(TargetSlot))
	{
		UnequipItem(TargetSlot);
	}

	if (!Inventory->RemoveItemFromSlot(InvSlotIndex, 1))
	{
		return false;
	}

	EquippedItems.Add(TargetSlot, ItemID);
	ModifyStatBonuses(*Data, 1.0f);
	UpdateVisualMesh(TargetSlot, Data);

	OnEquipmentChanged.Broadcast(TargetSlot);
	return true;
}

// 아이템 ID로 장착
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
	
	if (!IsSlotEmpty(TargetSlot))
	{
		UnequipItem(TargetSlot);
	}

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
	
	EquippedItems.Add(TargetSlot, ItemID);

	ModifyStatBonuses(*Data, 1.0f);
	
	UpdateVisualMesh(TargetSlot, Data);

	OnEquipmentChanged.Broadcast(TargetSlot);
	return true;
}

// 장비 해제
void UOHSMEquipmentComponent::UnequipItem(EEquipmentSlot Slot)
{
	FName* EquippedID = EquippedItems.Find(Slot);
	if (!EquippedID || EquippedID->IsNone())
	{
		return;
	}

	FName ItemID = *EquippedID;
	const FItemData* Data = FindItemData(ItemID);
	
	if (Data)
	{
		ModifyStatBonuses(*Data, -1.0f);
	}
	
	UpdateVisualMesh(Slot, nullptr);
	
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (Player)
	{
		UOHSMInventoryComponent* Inventory = Player->GetInventoryComponent();
		if (Inventory)
		{
			Inventory->AddItem(ItemID, 1, true);
		}
	}
	
	EquippedItems.Remove(Slot);

	OnEquipmentChanged.Broadcast(Slot);
}

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

// 스탯 보너스 적용/제거 (Multiplier: 1=추가, -1=제거)
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

// 장비 비주얼 메시 갱신
void UOHSMEquipmentComponent::UpdateVisualMesh(EEquipmentSlot Slot, const FItemData* Data)
{
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwner());
	if (!Player)
	{
		return;
	}

	if (Slot == EEquipmentSlot::Weapon)
	{
		AOHSMWeaponBase* CurrentWeapon = Player->GetCurrentWeapon();
		UStaticMeshComponent* WeaponMesh = CurrentWeapon ? CurrentWeapon->GetWeaponMesh() : nullptr;

		UE_LOG(LogTemp, Warning, TEXT("[EquipComp] UpdateVisualMesh - CurrentWeapon:%s  WeaponMesh:%s  Data:%s  ItemMesh:%s"),
			CurrentWeapon  ? TEXT("OK") : TEXT("NULL"),
			WeaponMesh     ? TEXT("OK") : TEXT("NULL"),
			Data           ? TEXT("OK") : TEXT("NULL"),
			(Data && Data->ItemMesh) ? *Data->ItemMesh->GetName() : TEXT("NULL"));

		if (WeaponMesh)
		{
			WeaponMesh->SetStaticMesh(Data ? Data->ItemMesh : nullptr);
		}
	}
}
