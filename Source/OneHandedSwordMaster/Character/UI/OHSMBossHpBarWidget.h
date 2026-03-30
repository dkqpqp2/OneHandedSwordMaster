// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMBossHpBarWidget.generated.h"

/**
 * 로스트아크 스타일 보스 체력바
 * - 화면 상단 중앙에 표시
 * - NativeTick에서 주기적으로 근처 보스 탐색
 * - 보스 발견 시 HealthComponent에 자동 바인딩
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBossHpBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

// ─── BindWidget ────────────────────────────────────────────────────────────
protected:
	// 보스 체력바 (필수 - 이름 정확히 일치)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> BossHpProgressBar;

	// 보스 이름 텍스트 (필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextBossName;

	// 현재HP / 최대HP 숫자 텍스트 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextBossHp;

// ─── 설정값 ────────────────────────────────────────────────────────────────
protected:
	// 이 거리 이내에서 보스 체력바 표시 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	float BossHpBarRange = 5000.0f;

	// 보스 탐색 주기 (초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	float SearchInterval = 0.3f;

// ─── 내부 상태 ────────────────────────────────────────────────────────────
private:
	// 현재 추적 중인 보스
	UPROPERTY()
	TObjectPtr<class AOHSMEnemyBase> CurrentBoss;

	float SearchTimer = 0.0f;

	// 주기적으로 가장 가까운 보스 탐색
	void FindNearestBoss();

	// 보스 변경 (nullptr이면 숨김)
	void SetCurrentBoss(class AOHSMEnemyBase* NewBoss);

	// HealthComponent 델리게이트 콜백
	UFUNCTION()
	void OnBossHpChanged(float CurrentHp, float MaxHp, float Damage, AActor* DamageCauser);

	UFUNCTION()
	void OnBossDied(AActor* Killer);
};
