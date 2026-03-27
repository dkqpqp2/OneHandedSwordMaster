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
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<class UOHSMInventoryWidget> InventoryWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<class UOHSMInventoryWidget> InventoryWidget;
	
protected:
	void InitializeHUDWidget();
	
	void InitializeInventoryWidget();
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void ToggleInventory();
	
	UFUNCTION(BlueprintPure, Category = "UI")
	UOHSMInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }
    
	UFUNCTION(BlueprintPure, Category = "UI")
	UOHSMHUDWidget* GetHUDWidget() const { return OHSMHUDWidget; }

protected:
	UPROPERTY()
	bool bIsInventoryOpen = false;

public:
	UFUNCTION(BlueprintPure, Category = "UI|Inventory")
	bool IsInventoryOpen() const { return bIsInventoryOpen; }
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetIsInventoryOpen(bool bOpen) { bIsInventoryOpen = bOpen; }
};
