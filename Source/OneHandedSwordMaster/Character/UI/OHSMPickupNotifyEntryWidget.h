// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMPickupNotifyEntryWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * 아이템 획득 알림 개별 항목
 * - 아이콘, 이름, 개수 표시
 * - DisplayTime 후 FadeOutTime 동안 서서히 사라짐
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMPickupNotifyEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 최초 생성 시 초기화
	void InitializeEntry(const FItemData& ItemData, int32 Count);

	// 같은 아이템 추가 획득 시 개수 갱신
	void AddCount(int32 AdditionalCount);

	// 이 항목의 ItemID 반환
	FName GetItemID() const { return ItemID; }

	// 완전히 사라졌는지 여부
	bool IsExpired() const { return bExpired; }

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// Blueprint에서 이름 맞춰서 배치
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemCountText;

	// 화면에 표시되는 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	float DisplayTime = 3.0f;

	// 페이드아웃 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	float FadeOutTime = 1.0f;

	// 페이드아웃 중 위로 올라가는 거리 (픽셀)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	float MoveUpDistance = 40.0f;

private:
	FName ItemID;
	int32 TotalCount = 0;
	float ElapsedTime = 0.0f;
	bool bExpired = false;
};
