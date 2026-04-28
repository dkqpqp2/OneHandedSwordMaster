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

	/**
	 * 허용된 최대 콤보 타격 수.
	 * 기본값 1 = 1타만 가능. 패시브 스킬 습득 시 증가.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 MaxComboCount = 1;
	
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

	// 공격 시작 전 이동 속도 저장 (몽타주 종료 후 복원용)
	float CachedMaxWalkSpeed    = 500.0f;
	float CachedMaxAcceleration = 2048.0f;
	
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

	/** 패시브 스킬 습득 시 호출 — MaxComboCount 를 갱신 (현재값보다 클 때만 적용) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetMaxComboCount(int32 Count);

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetMaxComboCount() const { return MaxComboCount; }
	
protected:
	void InitializeComboRows();
	
	void ResetCombo();
	
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
