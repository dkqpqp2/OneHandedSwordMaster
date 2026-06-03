// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMQuickSlotWidget.generated.h"

UENUM(BlueprintType)
enum class EQuickSlotGroupType : uint8
{
	Potion	UMETA(DisplayName = "포션 슬롯 (1~4)"),
	Skill	UMETA(DisplayName = "스킬 슬롯 (Q/E/R/F)")
};

// 퀵슬롯 4칸 컨테이너 (Potion/Skill 그룹 선택)
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯 초기화 (스킬 그룹이면 InSkillComp 전달)
	void InitializeSlots(
		class UOHSMQuickSlotComponent* InQuickSlotComp,
		class UOHSMInventoryComponent* InInventoryComp,
		class UOHSMSkillComponent*     InSkillComp = nullptr);

	void RefreshAllSlots();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMQuickSlotEntryWidget> QuickSlotEntry_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMQuickSlotEntryWidget> QuickSlotEntry_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMQuickSlotEntryWidget> QuickSlotEntry_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMQuickSlotEntryWidget> QuickSlotEntry_3;

	// 에디터에서 Potion / Skill 중 선택
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	EQuickSlotGroupType SlotGroupType = EQuickSlotGroupType::Potion;

protected:
	virtual void NativeDestruct() override;

	// OnInventoryUpdated 델리게이트에 바인딩할 수신자
	UFUNCTION()
	void OnInventoryChanged(int32 ChangedSlotIndex);

	// 스킬 슬롯 전용 — 스킬 사용 시 해당 슬롯에 쿨다운 시작
	void OnSkillActivated(FName SkillID, float Cooldown);

private:
	// SlotGroupType에 따라 컴포넌트 내 시작 인덱스 반환
	int32 GetBaseSlotOffset() const;

	TArray<class UOHSMQuickSlotEntryWidget*> GetAllEntries() const;

	UPROPERTY()
	TObjectPtr<class UOHSMSkillComponent> SkillComponent;

	UPROPERTY()
	TObjectPtr<class UOHSMQuickSlotComponent> QuickSlotComponent;

	/** OnInventoryUpdated 구독 해제용 캐시 — RemoveAll 에 사용 */
	UPROPERTY()
	TObjectPtr<class UOHSMInventoryComponent> InventoryComponent;

	static const TArray<FString> PotionKeyLabels;
	static const TArray<FString> SkillKeyLabels;
};
