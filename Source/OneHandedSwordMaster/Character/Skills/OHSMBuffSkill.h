// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OHSMSkillBase.h"
#include "OneHandedSwordMaster/Data/OHSMPlayerStatData.h"
#include "OHSMBuffSkill.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

/**
 * 버프 스킬 — 일정 시간 동안 지정한 스탯을 증가시킨다.
 * EffectValue : DataTable에서 설정 (버프 수치)
 * Duration    : DataTable에서 설정 (지속 시간, 초)
 * BuffStatType: DataTable에서 설정 (어느 스탯을 올릴지)
 * → Blueprint 하나로 체력/공격력/방어력 버프 모두 처리 가능
 */
UCLASS(Blueprintable)
class ONEHANDEDSWORDMASTER_API UOHSMBuffSkill : public UOHSMSkillBase
{
	GENERATED_BODY()

public:
	virtual void ActivateSkill_Implementation(ACharacter* Instigator) override;

protected:
	/** 버프 발동 시 재생할 몽타주 (없으면 즉시 적용) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Buff")
	TObjectPtr<UAnimMontage> BuffMontage;

	/** 몽타주 재생 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Buff")
	float PlayRate = 1.f;

	/**
	 * 버프 지속 중 캐릭터에 붙어 루프 재생되는 이펙트.
	 * 버프 만료 시 자동으로 제거된다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|FX")
	TObjectPtr<UNiagaraSystem> BuffLoopEffect;

	/** 루프 이펙트를 붙일 소켓 이름 (NAME_None 이면 루트에 Attach) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|FX")
	FName BuffLoopSocketName = NAME_None;

private:
	FTimerHandle BuffTimerHandle;
	TWeakObjectPtr<ACharacter> BuffedCharacter;
	float CachedEffectValue = 0.f;
	EPlayerStatType CachedStatType = EPlayerStatType::Damage;

	/** 현재 재생 중인 루프 이펙트 컴포넌트 (만료 시 제거) */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveLoopComponent;

	void OnBuffExpired();
};
