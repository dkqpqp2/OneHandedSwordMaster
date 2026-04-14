// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OneHandedSwordMaster/Character/UI/OHSMDamageNumberWidget.h"
#include "OHSMHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged,
	float, CurrentHealth,
	float, MaxHealth,
	float, Damage,
	AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ONEHANDEDSWORDMASTER_API UOHSMHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOHSMHealthComponent();

protected:
	virtual void BeginPlay() override;

// 체력
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	float MaxHealth = 100.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;
	
// 델리게이트
public:
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;
	
public:
	/** 데미지 숫자 위젯 클래스 — 에디터에서 WBP_DamageNumber 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Health|UI")
	TSubclassOf<UOHSMDamageNumberWidget> DamageNumberWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float Damage, AActor* DamageCauser);
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);
	
	// 현재 체력 가져오기
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }
	
	// 최대 체력 가져오기
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	// 체력 퍼센트
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return CurrentHealth / MaxHealth; }
	
	// 죽음 정보
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }
	
protected:
	void Die(AActor* Killer);
};
