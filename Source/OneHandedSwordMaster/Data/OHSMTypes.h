#pragma once

#include "CoreMinimal.h"
#include "OHSMTypes.generated.h"

UENUM(BlueprintType)
enum class EOHSMDamageType : uint8
{
	Physical UMETA(DisplayName = "물리"),
	Magic UMETA(DisplayName = "마법")
};

USTRUCT(BlueprintType)
struct FOHSMHitData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintreadWrite)
	float Damage = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	EOHSMDamageType DamageType = EOHSMDamageType::Physical;
	
	UPROPERTY(BlueprintReadWrite)
	FVector HitLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	FVector HitDirection = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	AActor* Attacker = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	bool bCaBeBlocked = true;
	
	UPROPERTY(BlueprintReadWrite)
	float KnockBackStrength = 0.0f;
};

USTRUCT(BlueprintType)
struct FOHSMCharacterStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Strength = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Dexterity = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Constitution = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Intelligence = 10;
	
	float GetPhysicalDamageMultiplier() const
	{
		return 1.0f + (Strength * 0.02f);
	}
	
	float GetMaxHealth() const 
	{ 
		return 100.0f + (Constitution * 5.0f); 
	}
};