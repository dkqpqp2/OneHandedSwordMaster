// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMSkillQuickSlotEntryWidget.generated.h"

/**
 * 스킬 퀵슬롯 단일 슬롯 위젯
 *
 * [Blueprint BindWidget]
 *   Img_SkillIcon      - 스킬 아이콘 이미지
 *   Img_CooldownOverlay- 쿨다운 원형 오버레이 (M_RadialCooldown 머티리얼 사용)
 *   Text_KeyLabel      - 단축키 라벨 (F1 ~ F4)
 *   HoverHighlight     - 마우스 오버 강조
 *
 * - 스킬 패널에서 드래그한 UOHSMSkillDragOperation 을 받아 슬롯에 등록
 * - 슬롯끼리 드래그해 순서 교체 가능
 * - 우클릭으로 슬롯 해제
 * - StartCooldown(Duration) 호출 시 Img_CooldownOverlay 가 시계 방향으로 사라짐
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMSkillQuickSlotEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @param InSkillSlotIndex  스킬 슬롯 인덱스 (0~3)
	 * @param InKeyLabel        표시할 단축키 문자열 (F1 등)
	 * @param InQuickSlotComp   QuickSlotComponent
	 * @param InSkillComp       SkillComponent (스킬 데이터 조회용)
	 */
	void InitializeEntry(int32 InSkillSlotIndex, const FString& InKeyLabel,
		class UOHSMQuickSlotComponent* InQuickSlotComp,
		class UOHSMSkillComponent* InSkillComp);

	/** 슬롯 아이콘 갱신 */
	void RefreshSlot();

	/** 현재 슬롯에 등록된 스킬 ID (없으면 NAME_None) */
	FName GetAssignedSkillID() const;

	/** 슬롯 인덱스 반환 (0~3) */
	int32 GetSkillSlotIndex() const { return SkillSlotIndex; }

	/**
	 * 쿨다운 시작 — Img_CooldownOverlay 가 시계 방향으로 사라짐
	 * @param CooldownDuration  쿨다운 총 시간 (초)
	 */
	void StartCooldown(float CooldownDuration);

protected:
	// ─── BindWidget ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_SkillIcon;

	/** 쿨다운 원형 스윕 오버레이 — M_RadialCooldown 머티리얼 적용 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_CooldownOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_KeyLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> HoverHighlight;

	// ─── 쿨다운 머티리얼 ─────────────────────────────────────────
	/**
	 * BP 디폴트에서 M_RadialCooldown 머티리얼을 지정.
	 * 머티리얼은 스칼라 파라미터 "Progress" (0 ~ 1) 를 가져야 함.
	 * Progress = 1 : 전체 원이 회색 (쿨다운 직후)
	 * Progress = 0 : 원 없음 (쿨다운 완료)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	TObjectPtr<class UMaterialInterface> CooldownMaterial;

protected:
	// ─── 드래그 & 드롭 ───────────────────────────────────────────
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	/** 스킬 슬롯 인덱스 (0~3) */
	int32 SkillSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<class UOHSMQuickSlotComponent> QuickSlotComponent;

	UPROPERTY()
	TObjectPtr<class UOHSMSkillComponent> SkillComponent;

	/** Img_CooldownOverlay 에 적용되는 동적 머티리얼 인스턴스 */
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> CooldownMID;

	FTimerHandle CooldownTimerHandle;
	float CooldownTotal   = 0.f;
	float CooldownElapsed = 0.f;

	/** 0.05 초마다 호출 — Progress 파라미터 갱신 */
	UFUNCTION()
	void OnCooldownTick();

	/** QuickSlotComponent 상의 실제 인덱스 (PotionSlotCount + SkillSlotIndex) */
	int32 GetComponentSlotIndex() const;
};
