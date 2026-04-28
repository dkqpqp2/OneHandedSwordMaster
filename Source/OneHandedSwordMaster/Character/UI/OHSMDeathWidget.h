#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMDeathWidget.generated.h"

/** 부활 포션 사용 버튼 클릭 시 */
DECLARE_MULTICAST_DELEGATE(FOnRevivalPotionRequested);

/** 마을에서 부활하기 버튼 클릭 시 */
DECLARE_MULTICAST_DELEGATE(FOnRespawnAtTownRequested);

/**
 * 플레이어 사망 화면.
 * - BtnRevivalPotion   : "부활물약 사용하기" (BindWidgetOptional, 포션 없으면 숨김)
 * - BtnRespawnAtTown   : "마을에서 부활하기" (BindWidgetOptional, 항상 표시)
 * - TextCountdown      : "N초 후 자동 부활" (BindWidget)
 * - ProgressCountdown  : 카운트다운 프로그레스 바 (BindWidgetOptional)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMDeathWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnRevivalPotionRequested OnRevivalPotionRequested;
	FOnRespawnAtTownRequested OnRespawnAtTownRequested;

	/** 카운트다운 수치 갱신 */
	void UpdateCountdown(float Remaining, float Total);

	/** 부활 포션 버튼 상태 갱신 (항상 표시, 개수 0이면 비활성화) */
	void UpdateRevivalButton(int32 PotionCount);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> BtnRevivalPotion;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> BtnRespawnAtTown;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextCountdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UProgressBar> ProgressCountdown;

	/** 보유 부활 포션 개수 표시 (예: "3개") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextPotionCount;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnRevivalButtonClicked();

	UFUNCTION()
	void OnRespawnAtTownButtonClicked();
};
