// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMEquipmentWidget.generated.h"

/**
 * 장비창 전체 위젯
 * - 5개 슬롯 (무기/투구/갑옷/바지/신발)
 * - PlayerController에서 열기/닫기
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMEquipmentWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

// ─── 스탯 표시 BindWidget ──────────────────────────────────
protected:
	/** 현재 레벨 표시 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextLevel;

	/** 공격력 표시 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextAtk;

	/** 방어력 표시 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextDef;

	/** 최대 체력 표시 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextHp;

	/** 최대 마나 표시 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextMp;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMEquipmentSlotWidget> EquipmentSlot_Weapon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMEquipmentSlotWidget> EquipmentSlot_Helmet;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMEquipmentSlotWidget> EquipmentSlot_Armor;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMEquipmentSlotWidget> EquipmentSlot_Pants;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMEquipmentSlotWidget> EquipmentSlot_Shoes;

public:
	void InitializeEquipment(class UOHSMEquipmentComponent* EquipComp,
		class UOHSMInventoryComponent* InvComp,
		class UOHSMPlayerStatComponent* InStatComp);

	/** 스탯 수치 갱신 (장비 변경 시) */
	UFUNCTION()
	void RefreshStats(EEquipmentSlot ChangedSlot);

	/** 스탯 수치 갱신 (레벨업 시) */
	UFUNCTION()
	void OnLevelUp(int32 NewLevel, int32 OldLevel);

	void UpdateStatsDisplay();

	void OpenEquipment();
	void CloseEquipment();
	void ToggleEquipment();

private:
	UPROPERTY()
	TObjectPtr<class UOHSMPlayerStatComponent> StatComponent;
};
