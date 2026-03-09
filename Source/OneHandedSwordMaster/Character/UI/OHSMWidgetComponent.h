// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "OHSMWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
protected:
	virtual void InitWidget() override;
};
