// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OHSMCombatData.generated.h"

UENUM(BlueprintType)
enum class EOHSMActionType : uint8
{
	BasicAttack UMETA(DisplayName = "기본공격"),
	Dodge		UMETA(DisplayName = "회피"),
	Block		UMETA(DisplayName = "막기"),
	Skill		UMETA(DisplayName = "스킬")
};


USTRUCT(BlueprintType)
struct FOHSMComboAnimData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 액션 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	EOHSMActionType ActionType = EOHSMActionType::BasicAttack;
	
	// 사용할 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	TObjectPtr<UAnimMontage> AnimationMontage = nullptr;
	
	// 다음 콤보 입력 가능 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	float ComboInputTime = 1.0f;
	
	// 콤보 인덱스 (순서)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "0"))
	int32 ComboIndex = 0;
	
	// RootMotion 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	bool bEnableRootMotion = true;
	
	// 데미지 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	float DamageMultiplier = 1.0f;
	
	FOHSMComboAnimData()
		: ActionType(EOHSMActionType::BasicAttack)
		, AnimationMontage(nullptr)
		, ComboInputTime(1.0f)
		, ComboIndex(0)
		, bEnableRootMotion(true)
		, DamageMultiplier(1.0f)
	{
	}
};
