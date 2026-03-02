// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMEnemyBase.h"
#include "OHSMEnemyRed.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMEnemyRed : public AOHSMEnemyBase
{
	GENERATED_BODY()
	
public:
	AOHSMEnemyRed();
	
protected:
	TObjectPtr<class UOHSMEnemyAnimInstance> EnemyAnimInst;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<class USphereComponent> RSocketCollision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<class USphereComponent> LSocketCollision;
	
public:
	virtual void ChangeAIAnimType(uint8 AnimType) override;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetRSocketCollisionEnabled(bool bEnable);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetLSocketCollisionEnabled(bool bEnable);
	
	UFUNCTION()
	void OnCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
};
