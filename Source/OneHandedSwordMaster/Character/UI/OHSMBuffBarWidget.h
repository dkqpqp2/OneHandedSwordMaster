// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMBuffBarWidget.generated.h"

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBuffBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeBar(class UOHSMSkillComponent* InSkillComp);

protected:
	virtual void NativeDestruct() override;

protected:
	// ─── BindWidget ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> Box_Buffs;

	/** BP 에서 WBP_BuffStatus 를 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	TSubclassOf<class UOHSMBuffStatusWidget> BuffStatusWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<class UOHSMSkillComponent> SkillComponent;

	/** 현재 표시 중인 버프 위젯 (SkillID → Widget) */
	UPROPERTY()
	TMap<FName, TObjectPtr<class UOHSMBuffStatusWidget>> ActiveBuffWidgets;

	/** OHSMSkillComponent::OnBuffApplied 수신 */
	void OnBuffApplied(FName SkillID, UTexture2D* Icon, float Duration);

	/** OHSMSkillComponent::OnBuffExpired 수신 */
	void OnBuffExpired(FName SkillID);
};
