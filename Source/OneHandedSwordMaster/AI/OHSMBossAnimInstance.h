// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OneHandedSwordMaster/AI/OHSMEnemyAnimInstance.h"
#include "OHSMBossAnimInstance.generated.h"

/**
 * 보스 전용 AnimInstance.
 * OHSMEnemyAnimInstance 를 상속하여 기존 Notify 핸들러를 모두 유지하면서
 * 보스 고유 Notify 핸들러를 추가한다.
 *
 * 보스 ABP(ABP_Boss)에서 AnimInstance Class 를 이 클래스로 지정할 것.
 *
 * ■ 추가된 AnimNotify 핸들러
 *   - AnimNotify_BossJump       : TriggerJumpAttack()
 *   - AnimNotify_BossAreaAttack : TriggerAreaAttack()
 *   - AnimNotify_BossGroundSlam : TriggerGroundSlam()
 *
 * ■ 상속된 AnimNotify 핸들러 (OHSMEnemyAnimInstance)
 *   - AnimNotify_OnCollision     : SetAttackCollisionEnabled(true)
 *   - AnimNotify_OffCollision    : SetAttackCollisionEnabled(false)
 *   - AnimNotify_SpawnProjectile : LaunchSkillProjectile()
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBossAnimInstance : public UOHSMEnemyAnimInstance
{
	GENERATED_BODY()

protected:
	/** 보스가 타겟 위치로 도약 (AnimNotify_BossJump) */
	UFUNCTION()
	void AnimNotify_BossJump();

	/** 보스 주변 AoE 에너지 방출 (AnimNotify_BossAreaAttack) */
	UFUNCTION()
	void AnimNotify_BossAreaAttack();

	/** 보스 전방 지면 강타 (AnimNotify_BossGroundSlam) */
	UFUNCTION()
	void AnimNotify_BossGroundSlam();
};
