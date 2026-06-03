// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMSkillQuickSlotEntryWidget.generated.h"

// 스킬 퀵슬롯 단일 슬롯 위젯
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMSkillQuickSlotEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯 초기화 (인덱스, 단축키 라벨)
	void InitializeEntry(int32 InSkillSlotIndex, const FString& InKeyLabel,
		class UOHSMQuickSlotComponent* InQuickSlotComp,
		class UOHSMSkillComponent* InSkillComp);

	// 슬롯 아이콘 갱신
	void RefreshSlot();

	// 등록된 스킬 ID 반환 (없으면 NAME_None)
	FName GetAssignedSkillID() const;

	// 슬롯 인덱스 반환
	int32 GetSkillSlotIndex() const { return SkillSlotIndex; }

	// 쿨다운 시작 (원형 오버레이 감소)
	void StartCooldown(float CooldownDuration);

protected:
	// ─── BindWidget ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_SkillIcon;

	// 쿨다운 원형 오버레이
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_CooldownOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_KeyLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> HoverHighlight;

	// ─── 쿨다운 머티리얼 ─────────────────────────────────────────
	// 쿨다운 머티리얼 (BP 지정, Progress 파라미터 사용)
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
	// 스킬 슬롯 인덱스
	int32 SkillSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<class UOHSMQuickSlotComponent> QuickSlotComponent;

	UPROPERTY()
	TObjectPtr<class UOHSMSkillComponent> SkillComponent;

	// 동적 머티리얼 인스턴스
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> CooldownMID;

	FTimerHandle CooldownTimerHandle;
	float CooldownTotal   = 0.f;
	float CooldownElapsed = 0.f;

	// 쿨다운 틱 (Progress 갱신)
	UFUNCTION()
	void OnCooldownTick();

	// 컴포넌트 실제 슬롯 인덱스 반환
	int32 GetComponentSlotIndex() const;
};
