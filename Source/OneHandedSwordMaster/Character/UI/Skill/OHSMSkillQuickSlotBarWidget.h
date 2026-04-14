// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMSkillQuickSlotBarWidget.generated.h"

/**
 * 스킬 퀵슬롯 바 (4칸)
 *
 * [Blueprint BindWidget]
 *   SkillSlot_0 ~ SkillSlot_3 : UOHSMSkillQuickSlotEntryWidget
 *
 * - InitializeBar 호출 시 각 슬롯 초기화 + OnSkillActivated 바인딩
 * - SkillComponent 에서 스킬 발동 신호를 받아 해당 슬롯의 쿨다운 UI 시작
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMSkillQuickSlotBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeBar(
		class UOHSMQuickSlotComponent* InQuickSlotComp,
		class UOHSMSkillComponent*     InSkillComp);

	/** 모든 슬롯 아이콘 갱신 */
	void RefreshAllSlots();

protected:
	virtual void NativeDestruct() override;

protected:
	// ─── BindWidget ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMSkillQuickSlotEntryWidget> SkillSlot_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMSkillQuickSlotEntryWidget> SkillSlot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMSkillQuickSlotEntryWidget> SkillSlot_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMSkillQuickSlotEntryWidget> SkillSlot_3;

private:
	UPROPERTY()
	TObjectPtr<class UOHSMSkillComponent> SkillComponent;

	/** 모든 엔트리를 배열로 반환 (반복 처리용) */
	TArray<class UOHSMSkillQuickSlotEntryWidget*> GetAllEntries() const;

	/** SkillComponent::OnSkillActivated 수신 — 해당 SkillID 슬롯의 쿨다운 시작 */
	void OnSkillActivated(FName SkillID, float CooldownDuration);
};
