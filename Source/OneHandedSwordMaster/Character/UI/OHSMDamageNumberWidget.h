// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMDamageNumberWidget.generated.h"

/**
 * 몬스터 피격 시 화면에 떠오르는 데미지 숫자 위젯.
 * ShowDamage() 호출 후 자동으로 위로 떠오르며 서서히 사라진다.
 *
 * [BindWidget]
 *   TextDamage — 데미지 수치 TextBlock
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 데미지 숫자를 표시하고 애니메이션을 시작한다.
	 * @param Amount        표시할 데미지 수치
	 * @param InPC          월드→화면 좌표 변환에 사용할 PlayerController
	 * @param InWorldLocation 데미지가 발생한 월드 좌표
	 */
	void ShowDamage(float Amount, APlayerController* InPC, FVector InWorldLocation);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextDamage;

	// ─── 설정 ─────────────────────────────────────────────────────
	/** 위로 떠오르는 총 거리 (px) */
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	float FloatDistance = 80.f;

	/** 전체 애니메이션 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	float Duration = 1.2f;

	/** 색상 — 일반 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	FLinearColor NormalColor = FLinearColor(1.f, 0.9f, 0.1f, 1.f);  // 노란색

private:
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPC;

	FVector WorldLocation = FVector::ZeroVector;
	float   ElapsedTime   = 0.f;
	bool    bStarted      = false;
};
