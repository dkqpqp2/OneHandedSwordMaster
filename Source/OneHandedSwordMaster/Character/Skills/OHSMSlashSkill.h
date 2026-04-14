// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMSkillBase.h"
#include "OHSMSlashSkill.generated.h"

class AOHSMSlashProjectile;

/**
 * 참격 스킬 — 캐릭터 전방으로 검기 투사체(AOHSMSlashProjectile)를 발사한다.
 * EffectValue : 투사체 데미지
 * SlashMontage: 발사 전 재생할 몽타주 (없으면 즉시 발사)
 */
UCLASS(Blueprintable)
class ONEHANDEDSWORDMASTER_API UOHSMSlashSkill : public UOHSMSkillBase
{
	GENERATED_BODY()

public:
	/** 발사할 투사체 Blueprint 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Slash")
	TSubclassOf<AOHSMSlashProjectile> ProjectileClass;

	/** 발사 전 재생할 몽타주 (선택) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Slash")
	TObjectPtr<UAnimMontage> SlashMontage;

	/** 캐릭터 전방에서 스폰할 오프셋 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Slash")
	float SpawnForwardOffset = 100.f;

	virtual void ActivateSkill_Implementation(ACharacter* Instigator) override;
};
