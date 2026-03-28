// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMPickupNotifyWidget.generated.h"

class UVerticalBox;
class UOHSMPickupNotifyEntryWidget;

/**
 * 아이템 획득 알림 컨테이너
 * - 획득한 아이템 목록을 VerticalBox에 쌓아서 표시
 * - 같은 아이템은 개수만 갱신
 * - 만료된 항목은 자동 제거
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMPickupNotifyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 아이템 획득 시 외부에서 호출
	void ShowItemPickup(const FItemData& ItemData, int32 Count);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// Blueprint에서 이름 맞춰서 배치
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> NotifyList;

	// 에디터에서 WBP_PickupNotifyEntry 지정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notify")
	TSubclassOf<UOHSMPickupNotifyEntryWidget> EntryWidgetClass;

	// 최대 동시 표시 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notify")
	int32 MaxEntries = 5;

private:
	UPROPERTY()
	TArray<TObjectPtr<UOHSMPickupNotifyEntryWidget>> Entries;
};
