// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMUserWidget.h"
#include "OHSMExpBar.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMExpBar : public UOHSMUserWidget
{
	GENERATED_BODY()
	

protected:
	virtual void NativeConstruct() override;
	
public:
	UFUNCTION()
	void UpdateExpBar(int32 CurrentExp, int32 RequiredExp);
	
	UPROPERTY()
	TObjectPtr<class UProgressBar> ExpProgressBar;
	
};
