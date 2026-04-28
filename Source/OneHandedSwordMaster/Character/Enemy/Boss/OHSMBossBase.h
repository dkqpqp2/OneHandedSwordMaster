// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "OneHandedSwordMaster/Data/OHSMBossData.h"
#include "OHSMBossBase.generated.h"

/** 페이즈 전환 시 Broadcast — HUD·BT Service 에서 구독 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, int32 /* NewPhase */);

/**
 * 보스 캐릭터 공통 기반 클래스.
 *
 * ■ 5가지 공격 패턴
 *   ① Normal      : 기본 펀치 — OnCollision / OffCollision AnimNotify로 데미지
 *   ② Projectile  : 바위 던지기 — AnimNotify_SpawnProjectile → LaunchSkillProjectile()
 *   ③ Jump        : 점프 공격 — AnimNotify_BossJump → TriggerJumpAttack() → Landed() AoE
 *   ④ Area        : 에너지 방출 — AnimNotify_BossAreaAttack → TriggerAreaAttack()
 *   ⑤ GroundSlam  : 전방 지면 강타 — AnimNotify_BossGroundSlam → TriggerGroundSlam()
 *
 * ■ 페이즈 시스템 (기본 3단계)
 *   Phase 1 : HP 100% ~ PhaseThresholds[0]% (기본 60%)
 *   Phase 2 : HP PhaseThresholds[0]% ~ PhaseThresholds[1]% (기본 30%)
 *   Phase 3 : HP PhaseThresholds[1]% 미만 → 광폭화(Enrage)
 *
 * ■ AnimNotify 목록 (ABP에서 사용)
 *   - AnimNotify_OnCollision     → SetAttackCollisionEnabled(true)
 *   - AnimNotify_OffCollision    → SetAttackCollisionEnabled(false)
 *   - AnimNotify_SpawnProjectile → LaunchSkillProjectile()   [바위 던지기]
 *   - AnimNotify_BossJump        → TriggerJumpAttack()       [점프 시작]
 *   - AnimNotify_BossAreaAttack  → TriggerAreaAttack()       [에너지 방출]
 *   - AnimNotify_BossGroundSlam  → TriggerGroundSlam()       [전방 강타]
 */
UCLASS(Abstract)
class ONEHANDEDSWORDMASTER_API AOHSMBossBase : public AOHSMEnemyBase
{
	GENERATED_BODY()

public:
	AOHSMBossBase();

protected:
	virtual void BeginPlay() override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

// ─── 페이즈 시스템 ────────────────────────────────────────────────────────
public:
	/** 현재 페이즈 (1 / 2 / 3) */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	/** 페이즈 전환 연출 재생 중이면 true */
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	bool IsPhaseChanging() const { return bIsPhaseChanging; }

	/**
	 * 페이즈 전환 연출(몽타주) 완료 후 BTTask_BossPhaseChange 에서 호출.
	 * AI 재개 + 광폭화(Phase 3) 처리.
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void FinishPhaseTransition();

	/** 페이즈 전환 몽타주 배열 (BTTask에서 참조) */
	const TArray<TObjectPtr<UAnimMontage>>& GetPhaseChangeMontages() const
	{
		return PhaseChangeMontages;
	}

	/** 페이즈 전환 델리게이트 */
	FOnBossPhaseChanged OnPhaseChanged;

protected:
	/**
	 * 페이즈 전환 HP 임계값 (0~1).
	 * 기본: {0.6f, 0.3f} → 60% 이하 Phase2, 30% 이하 Phase3
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TArray<float> PhaseThresholds = {0.6f, 0.3f};

	/**
	 * 페이즈 전환 연출 몽타주 배열.
	 * [0] = Phase1→Phase2 전환 시 재생
	 * [1] = Phase2→Phase3 전환 시 재생
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TArray<TObjectPtr<UAnimMontage>> PhaseChangeMontages;

	/** Phase3 돌입 시 이동 속도 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Enrage",
		meta = (ClampMin = "1.0"))
	float EnrageSpeedMultiplier = 1.5f;

	/** Phase3 돌입 시 데미지 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Enrage",
		meta = (ClampMin = "1.0"))
	float EnrageDamageMultiplier = 1.5f;

	int32 CurrentPhase    = 1;
	bool  bIsPhaseChanging = false;
	bool  bIsEnraged       = false;

	/** HP 비율로 다음 페이즈 여부 확인 후 전환 시작 */
	void CheckAndTransitionPhase();

	/** 페이즈 전환 시작 — AI 일시정지 + 델리게이트 브로드캐스트 */
	virtual void StartPhaseTransition(int32 NewPhase);

// ─── 공격 패턴 ────────────────────────────────────────────────────────────
public:
	/**
	 * 현재 페이즈에 맞는 공격 패턴 선택 (BTTask_BossAttack 에서 호출).
	 * 가중치 + 쿨다운 기반 랜덤. nullptr 반환 시 적절한 패턴 없음.
	 */
	const FOHSMBossAttackPattern* SelectBossAttackPattern();

	/** 패턴 쿨다운 등록 (SelectBossAttackPattern 내부에서 자동 호출) */
	void RegisterPatternCooldown(FName PatternID, float Cooldown);

	/** 현재 데미지 배율 (광폭화 상태면 EnrageDamageMultiplier 적용) */
	UFUNCTION(BlueprintPure, Category = "Boss|Combat")
	float GetDamageMultiplier() const { return bIsEnraged ? EnrageDamageMultiplier : 1.0f; }

	// ── 공통 유틸 ────────────────────────────────────────────────────────
	/**
	 * 공격 시작 전 TargetActor 방향으로 즉시 회전.
	 * BTTask_BossAttack 및 각 Trigger 함수에서 호출.
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void FaceTarget();

	// ── ① 기본 펀치 ─────────────────────────────────────────────────────
	// SetAttackCollisionEnabled() 오버라이드는 자식 BP/클래스에서 처리

	// ── ② 바위 던지기 ────────────────────────────────────────────────────
	/**
	 * AnimNotify_SpawnProjectile 에서 호출.
	 * RockProjectileClass 로 투사체를 스폰하고 타겟 방향으로 발사.
	 */
	virtual void LaunchSkillProjectile() override;

	// ── ③ 점프 공격 ──────────────────────────────────────────────────────
	/**
	 * AnimNotify_BossJump 에서 호출 — LaunchCharacter로 타겟 위치로 도약.
	 * 착지 시 Landed()에서 자동으로 AoE 데미지 적용.
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void TriggerJumpAttack();

	// ── ④ 에너지 방출 (주변 AoE) ─────────────────────────────────────────
	/**
	 * AnimNotify_BossAreaAttack 에서 호출 — 보스 중심 반경 AoE.
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	virtual void TriggerAreaAttack();

	// ── ⑤ 전방 지면 강타 ─────────────────────────────────────────────────
	/**
	 * AnimNotify_BossGroundSlam 에서 호출 — 전방 박스 범위 데미지.
	 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	virtual void TriggerGroundSlam();

protected:
	/** 1페이즈 공격 패턴 테이블 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	TObjectPtr<UDataTable> Phase1AttackTable;

	/** 2페이즈 공격 패턴 테이블 (없으면 Phase1 테이블 사용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	TObjectPtr<UDataTable> Phase2AttackTable;

	/** 3페이즈 공격 패턴 테이블 (없으면 Phase2 → Phase1 순으로 폴백) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	TObjectPtr<UDataTable> Phase3AttackTable;

	/** 현재 페이즈에 맞는 DataTable 반환 */
	UDataTable* GetCurrentPhaseAttackTable() const;

	/** 패턴 쿨다운 만료 여부 확인 */
	bool IsPatternOnCooldown(FName PatternID) const;

	/** 직전에 선택된 패턴 포인터 (각 Trigger 함수에서 참조) */
	const FOHSMBossAttackPattern* ActivePattern = nullptr;

	// ─── ② 바위 던지기 설정 ─────────────────────────────────────────────
	/** 스폰할 바위 투사체 클래스 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	TSubclassOf<class AOHSMBossRockProjectile> RockProjectileClass;

	/** 바위가 스폰될 소켓 이름 (예: "Hand_R" 또는 "Muzzle_Front") */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	FName RockSpawnSocket = TEXT("Hand_R");

	// ─── ③ 점프 공격 상태 ───────────────────────────────────────────────
	/** 점프 공격 도약 시 이펙트 (발 떼는 순간) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	TObjectPtr<class UNiagaraSystem> JumpStartEffect;

	/** 점프 공격 착지 시 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	TObjectPtr<class UNiagaraSystem> JumpLandEffect;

	/** 점프 공격 착지 시 카메라 셰이크 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	TSubclassOf<class UCameraShakeBase> JumpLandCameraShake;

	bool    bIsJumpAttacking     = false;
	FVector JumpAttackLandTarget = FVector::ZeroVector;
	FVector JumpAttackStartPos   = FVector::ZeroVector;
	float   JumpAttackElapsed    = 0.f;
	float   JumpAttackDuration   = 0.8f;

	/** 점프 호 높이 (cm) — BP에서 조정 가능 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	float JumpArcHeight = 500.f;

	// ─── ④ 에너지 방출 (AreaAttack) 확장 이펙트 ────────────────────────
	/** 에너지 방출 이펙트 — 0 에서 AreaRadius 까지 스케일 보간 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Area")
	TObjectPtr<class UNiagaraSystem> AreaAttackEffect;

	/** 이펙트가 최대 크기까지 퍼지는 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Area",
		meta = (ClampMin = "0.1"))
	float AreaExpandDuration = 0.8f;

	TObjectPtr<class UNiagaraComponent> ActiveAreaEffectComp;
	bool  bIsAreaExpanding    = false;
	float AreaExpandElapsed   = 0.f;
	float AreaExpandMaxRadius = 0.f;
	float AreaExpandDamage    = 0.f;

public:
	bool IsJumpAttacking() const { return bIsJumpAttacking; }

protected:
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * ActivePattern 기반 AoE 데미지 적용.
	 * PerformAttack() 대신 사용 — 보스는 CurrentAttackPattern 이 아닌 ActivePattern 을 씀.
	 */
	void ApplyBossAoeDamage(FVector Origin, float Radius, float Damage);

private:
	/** PatternID → 쿨다운 만료 시각 (GetWorld()->GetTimeSeconds() 기준) */
	TMap<FName, float> PatternCooldownMap;

public:
	/**
	 * 현재 상태(페이즈·쿨다운·거리)에서 사용 가능한 패턴이 하나라도 있는지 확인.
	 * BTTask_BossChase 에서 추적 중단 타이밍 판단에 사용.
	 */
	bool HasAnyAvailablePattern() const;
};
