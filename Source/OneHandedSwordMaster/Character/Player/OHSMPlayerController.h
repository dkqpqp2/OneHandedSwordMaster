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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<class UOHSMEquipmentWidget> EquipmentWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<class UOHSMEquipmentWidget> EquipmentWidget;

protected:
	void InitializeHUDWidget();
	void InitializeInventoryWidget();
	void InitializeEquipmentWidget();

	/** 현재 열린 창 상태에 따라 입력 모드 / 마우스 커서 자동 갱신 */
	void UpdateInputMode();

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI|Equipment")
	void ToggleEquipment();
	
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

	/** 인벤토리 또는 장비창 중 하나라도 열려있으면 true */
	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsAnyWindowOpen() const;
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetIsInventoryOpen(bool bOpen) { bIsInventoryOpen = bOpen; }

protected:
	UFUNCTION()
	void OnItemPickedUp(FName ItemID, int32 Count);
};
