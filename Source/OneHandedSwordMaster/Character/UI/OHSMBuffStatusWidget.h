// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMBuffStatusWidget.generated.h"

// 버프 1개 아이콘 + 남은 시간 표시 위젯
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBuffStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 버프 아이콘 초기화 (스킬ID, 아이콘, 지속시간)
	void InitBuff(FName InSkillID, UTexture2D* InIcon, float InDuration);

	// 현재 표시 중인 스킬 ID
	FName GetSkillID() const { return SkillID; }

	// 버프 강제 만료 (외부 호출)
	void ForceExpire();

protected:
	// ─── BindWidget ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_BuffIcon;

	// 라디얼 타이머 이미지 (Progress 1→0)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_TimerRadial;

	// 남은 시간 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Duration;

	// 라디얼 타이머 머티리얼 (BP 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Buff|FX")
	TObjectPtr<class UMaterialInterface> TimerMaterial;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	FName  SkillID;
	float  TotalDuration  = 0.f;
	float  RemainingTime  = 0.f;
	bool   bExpired       = false;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> TimerMID;
};
