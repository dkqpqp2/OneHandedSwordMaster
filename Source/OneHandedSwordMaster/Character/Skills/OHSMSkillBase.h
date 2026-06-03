// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OneHandedSwordMaster/Data/OHSMSkillData.h"
#include "OHSMSkillBase.generated.h"

class UNiagaraSystem;

// 스킬 기반 클래스 (쿨다운·마나는 SkillComponent 담당)
UCLASS(Abstract, Blueprintable, BlueprintType)
class ONEHANDEDSWORDMASTER_API UOHSMSkillBase : public UObject
{
	GENERATED_BODY()

public:
	// DataTable Row 이름
	FName SkillID;

	// 스킬 데이터 포인터 (컴포넌트에서 설정)
	const FOHSMSkillData* SkillData = nullptr;

	// 스킬 습득 시 호출 (패시브 영구 스탯 적용)
	UFUNCTION(BlueprintNativeEvent, Category = "Skill")
	void OnLearned(ACharacter* Instigator);
	virtual void OnLearned_Implementation(ACharacter* Instigator) {}

	// 스킬 발동 (서브클래스에서 구현)
	UFUNCTION(BlueprintNativeEvent, Category = "Skill")
	void ActivateSkill(ACharacter* Instigator);
	virtual void ActivateSkill_Implementation(ACharacter* Instigator) {}

	// ─── 쿨다운 ──────────────────────────────────────────────────
	// 쿨다운 종료 시각
	float CooldownEndTime = 0.f;

	bool IsOnCooldown(float CurrentTime) const { return CurrentTime < CooldownEndTime; }
	void StartCooldown(float CurrentTime, float Duration) { CooldownEndTime = CurrentTime + Duration; }

	// Outer(UActorComponent)를 통해 World 접근
	UWorld* GetWorld() const override;

protected:
	// 몽타주 재생 + 이동 잠금 (종료 시 자동 복구)
	void PlayMontageAndBlockMovement(ACharacter* Char, UAnimMontage* Montage, float InPlayRate = 1.f);

	// ─── 나이아가라 이펙트 ───────────────────────────────────────

	// 스킬 발동 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Skill|FX")
	TObjectPtr<UNiagaraSystem> ActivateEffect;

	// 이펙트 소켓 이름 (NAME_None이면 월드 스폰)
	UPROPERTY(EditDefaultsOnly, Category = "Skill|FX")
	FName EffectSocketName = NAME_None;

	// 발동 이펙트 스폰 헬퍼
	void SpawnActivateEffect(ACharacter* Char);

private:
	// 몽타주 종료 수신 (이동 복구)
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 이동 복구용 캐릭터 포인터
	UPROPERTY()
	TObjectPtr<ACharacter> CachedCharacter;

	// 감지 대상 몽타주 (다른 몽타주 무시)
	UPROPERTY()
	TObjectPtr<UAnimMontage> CachedMontage;
};
