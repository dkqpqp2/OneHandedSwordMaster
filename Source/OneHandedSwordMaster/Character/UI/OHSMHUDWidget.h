// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UOHSMHUDWidget(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void NativeConstruct() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMHpBar> WidgetHpBar; 
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextHp;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMManaBar> WidgetManaBar; 
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMana;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOHSMExpBar> WidgetExpBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextExp;

	// 아이템 획득 알림 위젯 (BindWidget이 아닌 BindWidgetOptional - 없어도 빌드 오류 X)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UOHSMPickupNotifyWidget> PickupNotifyWidget;

	// HP바 아래 포션 퀵슬롯 4칸 (SlotGroupType = Potion)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UOHSMQuickSlotWidget> WidgetPotionSlots;

	// MP바 아래 스킬 퀵슬롯 4칸 (SlotGroupType = Skill)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UOHSMQuickSlotWidget> WidgetSkillSlots;

	// 스킬 전용 퀵슬롯 바 — WBP_SkillQuickSlotBar 를 이 변수에 연결
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UOHSMSkillQuickSlotBarWidget> SkillQuickSlotBar;

	// 화면 좌상단 버프 상태 바 — WBP_BuffBar 를 이 변수에 연결
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UOHSMBuffBarWidget> BuffBar;

	// 화면 상단 보스 체력바
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UOHSMBossHpBarWidget> BossHpBar;

	// 화면 우측 퀘스트 트래커
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UOHSMQuestTrackerWidget> QuestTracker;

	void InitializeHUD();

public:
	/** 맵 전환 후 새 QuickSlotComp / SkillComp 에 재연결할 때 호출 */
	void InitializeQuickSlots();

public:
	UFUNCTION()
	void UpdateHp(float CurrentHp, float MaxHp);
    
	UFUNCTION()
	void UpdateMana(float CurrentMana, float MaxMana);  // ← 추가!
    
	UFUNCTION()
	void OnExpChanged(int32 CurrentExp, int32 RequiredExp);

	// 아이템 획득 알림 표시
	UFUNCTION()
	void ShowItemPickup(FName ItemID, int32 Count);
};
