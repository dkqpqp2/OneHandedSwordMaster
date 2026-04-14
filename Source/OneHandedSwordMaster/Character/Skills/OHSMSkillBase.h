// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OneHandedSwordMaster/Data/OHSMSkillData.h"
#include "OHSMSkillBase.generated.h"

class UNiagaraSystem;

/**
 * 스킬 실행 로직의 기반 클래스.
 * 서브클래스(버프/타격/참격)에서 ActivateSkill_Implementation을 재정의한다.
 * 쿨다운·마나 소모 처리는 UOHSMSkillComponent가 담당한다.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class ONEHANDEDSWORDMASTER_API UOHSMSkillBase : public UObject
{
	GENERATED_BODY()

public:
	/** DataTable Row Name — 컴포넌트에서 초기화 시 설정 */
	FName SkillID;

	/** 스킬 데이터 포인터 — 컴포넌트에서 설정 (DataTable 수명동안 유효) */
	const FOHSMSkillData* SkillData = nullptr;

	/** 스킬 습득 시 호출 — 패시브 스킬에서 영구 스탯 적용에 사용 */
	UFUNCTION(BlueprintNativeEvent, Category = "Skill")
	void OnLearned(ACharacter* Instigator);
	virtual void OnLearned_Implementation(ACharacter* Instigator) {}

	/** 스킬 발동 — 버프/타격/참격 서브클래스에서 구현 (패시브는 사용 불가) */
	UFUNCTION(BlueprintNativeEvent, Category = "Skill")
	void ActivateSkill(ACharacter* Instigator);
	virtual void ActivateSkill_Implementation(ACharacter* Instigator) {}

	// ─── 쿨다운 ──────────────────────────────────────────────────
	/** 쿨다운 종료 시각 (UWorld::GetTimeSeconds 기준) */
	float CooldownEndTime = 0.f;

	bool IsOnCooldown(float CurrentTime) const { return CurrentTime < CooldownEndTime; }
	void StartCooldown(float CurrentTime, float Duration) { CooldownEndTime = CurrentTime + Duration; }

	/** UObject는 World를 직접 알 수 없으므로 Outer(UActorComponent)를 통해 얻는다 */
	UWorld* GetWorld() const override;

protected:
	/**
	 * 몽타주를 재생하고, 몽타주가 끝날 때까지 캐릭터 이동을 막는다.
	 * Attack/Slash 서브클래스에서 Montage_Play 대신 이 함수를 사용한다.
	 *
	 * @param Char       대상 캐릭터
	 * @param Montage    재생할 AnimMontage
	 * @param InPlayRate 재생 속도 (기본 1.0)
	 */
	void PlayMontageAndBlockMovement(ACharacter* Char, UAnimMontage* Montage, float InPlayRate = 1.f);

	// ─── 나이아가라 이펙트 ───────────────────────────────────────

	/** 스킬 발동 시 1회 재생되는 이펙트 (캐스팅/타격 이펙트) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|FX")
	TObjectPtr<UNiagaraSystem> ActivateEffect;

	/**
	 * 이펙트를 붙일 소켓 이름
	 * - 값이 있으면 해당 소켓에 Attach (예: "hand_r", "root")
	 * - NAME_None 이면 캐릭터 위치에 월드 스폰
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|FX")
	FName EffectSocketName = NAME_None;

	/**
	 * 스킬 발동 이펙트 스폰 헬퍼.
	 * ActivateEffect 가 설정된 경우에만 스폰되며, EffectSocketName 유무에 따라
	 * Attach 또는 WorldLocation 스폰을 자동 선택한다.
	 */
	void SpawnActivateEffect(ACharacter* Char);

private:
	/** AnimInstance::OnMontageEnded 수신 — 이동 복구 후 델리게이트 해제 */
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 이동 복구를 위해 캐싱해두는 캐릭터 포인터 */
	UPROPERTY()
	TObjectPtr<ACharacter> CachedCharacter;

	/** 종료 감지 대상 몽타주 (다른 몽타주 종료 신호 무시용) */
	UPROPERTY()
	TObjectPtr<UAnimMontage> CachedMontage;
};
