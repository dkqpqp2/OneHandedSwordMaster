// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "OHSMTargetingIndicatorWidget.generated.h"

/**
 * 타겟 락온 마커 위젯
 * - NativeTick에서 타겟의 spine_02 소켓 위치를 스크린 좌표로 변환해 따라다님
 * - DPI 스케일 반영
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMTargetingIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTarget(AActor* InTarget);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> TargetMarker;

	// 소켓이 없을 때 머리 위 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float WorldHeightOffset = 120.0f;

	// 타겟 위치 소켓 이름 (블로그 방식: spine_02)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	FName SocketName = FName("spine_02");

private:
	UPROPERTY()
	TObjectPtr<AActor> Target;

	// spine_02 소켓 위치 반환 (없으면 머리 위 오프셋)
	FVector GetTargetWorldLocation() const;
};
