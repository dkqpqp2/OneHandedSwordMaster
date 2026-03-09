// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOwningActor(AActor* NewOwner) { OwningActor = NewOwner; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<AActor> OwningActor;
};
