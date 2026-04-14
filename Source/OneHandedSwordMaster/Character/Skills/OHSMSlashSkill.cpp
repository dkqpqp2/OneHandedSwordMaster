// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMSlashSkill.h"

#include "OHSMSlashProjectile.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

void UOHSMSlashSkill::ActivateSkill_Implementation(ACharacter* Instigator)
{
	if (!IsValid(Instigator) || !SkillData) return;
	if (!ProjectileClass) return;

	// 발동 이펙트 스폰 (캐스팅 이펙트)
	SpawnActivateEffect(Instigator);

	// 데미지 = 플레이어 공격력 × EffectValue 배율
	float AttackPower = SkillData->EffectValue;
	if (UOHSMPlayerStatComponent* StatComp =
		Instigator->FindComponentByClass<UOHSMPlayerStatComponent>())
	{
		AttackPower = StatComp->GetAttackPower() * SkillData->EffectValue;
	}

	// 투사체 정보를 캐릭터에 저장 — 실제 스폰은 AnimNotify_SpawnProjectile 에서 수행
	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(Instigator))
	{
		Player->PrepareProjectileSpawn(ProjectileClass, AttackPower, SpawnForwardOffset);
	}

	// 몽타주 재생 + 몽타주가 끝날 때까지 이동 잠금
	if (IsValid(SlashMontage))
	{
		PlayMontageAndBlockMovement(Instigator, SlashMontage);
	}
}
