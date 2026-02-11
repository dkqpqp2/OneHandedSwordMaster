// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"
#include "OHSMCombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ONEHANDEDSWORDMASTER_API UOHSMCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOHSMCombatComponent();

protected:
	virtual void BeginPlay() override;
	
// DataTable
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|DataTable")
	TObjectPtr<UDataTable> ComboDataTable;
	
	TArray<FOHSMComboAnimData*> BasicAttackRows;
	
	TArray<FOHSMComboAnimData*> DodgeRows;
	
	
// 콤보 상태
protected:
	// 현재 콤보 인데스
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 CurrentComboIndex = 0;
	
	// 다음 콤보 입력 가능 여부
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bCanNextCombo = false;
	
	// 입력 받을 수 있는지 (AnimNotify에서 제어)
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bCanReceiveInput = false;
	
	// 공격 중 
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;
	
	// 현재 재생 중인 몽타주
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> CurrentMontage;
	
	// 콤보 리셋 타이머
	FTimerHandle ComboResetTimerHandle;
	
protected:
	UPROPERTY()
	TObjectPtr<class ACharacter> OwnerCharacter;
	
	UPROPERTY()
	TObjectPtr<class UAnimInstance> AnimInstance;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformBasicAttack();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformDodge();
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }
	
public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnableNextAttackInput();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnableInputReceive();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DisableInputReceive();
	
protected:
	void InitializeComboRows();
	
	void ResetCombo();
	
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
