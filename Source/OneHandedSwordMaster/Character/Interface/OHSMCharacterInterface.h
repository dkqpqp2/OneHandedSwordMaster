// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OHSMCharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UOHSMCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ONEHANDEDSWORDMASTER_API IOHSMCharacterInterface
{
	GENERATED_BODY()

public:
	virtual void SetupCharacterWidget(class UOHSMUserWidget* InUserWidget) = 0;
};
