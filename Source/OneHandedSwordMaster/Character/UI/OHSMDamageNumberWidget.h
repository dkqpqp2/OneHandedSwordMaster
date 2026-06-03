// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMDamageNumberWidget.generated.h"

// 피격 시 화면에 뜨는 데미지 숫자 위젯
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 데미지 수치 표시 및 애니메이션 시작
	void ShowDamage(float Amount, APlayerController* InPC, FVector InWorldLocation);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextDamage;

	// ─── 설정 ─────────────────────────────────────────────────────
	// 위로 떠오르는 거리 (px)
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	float FloatDistance = 80.f;

	// 애니메이션 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	float Duration = 1.2f;

	// 데미지 텍스트 색상
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	FLinearColor NormalColor = FLinearColor(1.f, 0.9f, 0.1f, 1.f);  // 노란색

private:
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPC;

	FVector WorldLocation = FVector::ZeroVector;
	float   ElapsedTime   = 0.f;
	bool    bStarted      = false;
};
