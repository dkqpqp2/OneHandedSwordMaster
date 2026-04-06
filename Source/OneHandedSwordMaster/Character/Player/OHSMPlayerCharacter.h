// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMCharacterHUDInterface.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMCharacterInterface.h"
#include "OHSMPlayerCharacter.generated.h"

class UOHSMPlayerStatComponent;

UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMPlayerCharacter : public ACharacter, public IOHSMCharacterInterface, public IOHSMCharacterHUDInterface
{
	GENERATED_BODY()

public:
	AOHSMPlayerCharacter();
	
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;
	
	// 공격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InventoryAction;

	// 장비창
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> EquipmentAction;

	// 제작창
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> CraftAction;

	// 상호작용 (E키)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InteractAction;

	// 포션 퀵슬롯 단축키 (1, 2, 3, 4)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot4Action;
	
// ============= 무기 시스템 =============
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AOHSMWeaponBase> CurrentWeapon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AOHSMWeaponBase> WeaponClass;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(class AOHSMWeaponBase* Weapon);
	
	AOHSMWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
	
// 공격 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMCombatComponent> CombatComponent;
	
// 인벤토리 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMInventoryComponent> InventoryComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UOHSMInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

// 퀵슬롯 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMQuickSlotComponent> QuickSlotComponent;

public:
	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	UOHSMQuickSlotComponent* GetQuickSlotComponent() const { return QuickSlotComponent; }

// 장비 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMEquipmentComponent> EquipmentComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Equipment")
	class UOHSMEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

// 제작 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMCraftComponent> CraftComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Craft")
	class UOHSMCraftComponent* GetCraftComponent() const { return CraftComponent; }


protected:
	void ToggleInventory();
	
protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Attack();
	void SetDead();

	void UsePotionSlot1();
	void UsePotionSlot2();
	void UsePotionSlot3();
	void UsePotionSlot4();

	void ToggleEquipment();
	void ToggleCraftPanel();
	
public:
	TObjectPtr<class USpringArmComponent> GetCameraBoom() const { return CameraBoom; }
	TObjectPtr<class UCameraComponent> GetFollowCamera() const { return FollowCamera; }
	TObjectPtr<class UOHSMCombatComponent> GetCombatComponent() const { return CombatComponent; }
	
	UFUNCTION(BlueprintPure, Category = "Components")
	UOHSMPlayerStatComponent* GetStatComponent() const { return PlayerStat; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMPlayerStatComponent> PlayerStat;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMWidgetComponent> HpBar;
	
	virtual void SetupCharacterWidget(class UOHSMUserWidget* InUserWidget) override;
	virtual void SetupHUDWidget(class UOHSMHUDWidget* InHUDWidget) override;
	
protected:
	/** 경험치 추가 (테스트용) */
	UFUNCTION(Exec)
	void AddExp(int32 Amount);
	
};
