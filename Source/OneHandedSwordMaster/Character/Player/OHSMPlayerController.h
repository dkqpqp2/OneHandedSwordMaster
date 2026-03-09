// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OHSMPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AOHSMPlayerController();
	
protected:
	virtual void BeginPlay() override;
	
	// HUD 
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<class UOHSMHUDWidget> HUDWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	TObjectPtr<class UOHSMHUDWidget> OHSMHUDWidget;
	
};
