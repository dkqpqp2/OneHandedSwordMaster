// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OHSMWeaponBase.generated.h"

UENUM(BlueprintType)
enum class EOHSMWeaponType : uint8
{
	OneHandSword UMETA(DisplayName = "한손검"),
	TwoHandSword UMETA(DisplayName = "양손검"),
	Bow UMETA(DisplayName = "활"),
	Staff UMETA(DisplayName = "지팡이")
};

UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AOHSMWeaponBase();

protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BLueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UStaticMeshComponent> WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Trace", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USceneComponent> TraceStart;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Trace", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USceneComponent> TraceEnd;
	
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EOHSMWeaponType WeaponType = EOHSMWeaponType::OneHandSword;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FText WeaponName = FText::FromString(TEXT("Basic Sword"));
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	float BaseDamage = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	float AttackSpeedMultiplier = 1.0f;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapont|Socket")
	FName EquipSocketName = TEXT("WeaponSocket");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapont|Socket")
	FName SheathSocketName = TEXT("WeaponSheath");
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapont|Trace")
	float TraceRadius = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapont|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;
	
	// 이미 맞은 액터 목록 (중복 히트 방지)
	TSet<AActor*> HitActors;
	
	// Trace 활성화 여부
	bool bIsTracing = false;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipToCharacter(ACharacter* Character);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SheathToCharacter (ACharacter* Character);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartTracing();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopTracing();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void PerformTrace();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ClearHitActors();
	
public:
	float GetBaseDamage() const { return BaseDamage; }
	EOHSMWeaponType GetWeaponType() const { return WeaponType; }
	TObjectPtr<class UStaticMeshComponent> GetWeaponMesh() const { return WeaponMesh; }
	
protected:
	virtual void OnHitDetected(AActor* HitActor, const FHitResult& HitResult);
	
	AActor* GetWeaponOwner() const;
	
};