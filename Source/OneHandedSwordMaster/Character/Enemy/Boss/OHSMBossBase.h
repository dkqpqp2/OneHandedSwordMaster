// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "OneHandedSwordMaster/Data/OHSMBossData.h"
#include "OHSMBossBase.generated.h"

// 페이즈 전환 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, int32 /* NewPhase */);

// 보스 공통 기반 클래스 (페이즈 시스템 + 5가지 공격 패턴)
UCLASS(Abstract)
class ONEHANDEDSWORDMASTER_API AOHSMBossBase : public AOHSMEnemyBase
{
	GENERATED_BODY()

public:
	AOHSMBossBase();

protected:
	virtual void BeginPlay() override;

public:
	// 현재 페이즈 반환
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	// 페이즈 전환 중 여부
	UFUNCTION(BlueprintPure, Category = "Boss|Phase")
	bool IsPhaseChanging() const { return bIsPhaseChanging; }

	// 페이즈 전환 완료 처리
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void FinishPhaseTransition();

	// 페이즈 전환 몽타주 배열 (BTTask 참조용)
	const TArray<TObjectPtr<UAnimMontage>>& GetPhaseChangeMontages() const
	{
		return PhaseChangeMontages;
	}

	// 페이즈 전환 델리게이트
	FOnBossPhaseChanged OnPhaseChanged;

protected:
	// HP 임계값 배열 (50% → Phase 2 전환)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TArray<float> PhaseThresholds = {0.5f};

	// 페이즈 전환 몽타주 배열
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TArray<TObjectPtr<UAnimMontage>> PhaseChangeMontages;

	// 페이즈 전환 연출 몽타주 (주먹 올려치기 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TObjectPtr<UAnimMontage> PhaseChangeMontage;

	// 페이즈 전환 Niagara 이펙트 (hand_rSocket 에 부착)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	TObjectPtr<class UNiagaraSystem> PhaseChangeEffect;

	// 광폭화 이동속도 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Enrage",
		meta = (ClampMin = "1.0"))
	float EnrageSpeedMultiplier = 1.5f;

	// 광폭화 데미지 배율 (Phase 2 전환 시 적용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Enrage",
		meta = (ClampMin = "1.0"))
	float EnrageDamageMultiplier = 1.2f;

	int32 CurrentPhase    = 1;
	bool  bIsPhaseChanging = false;
	bool  bIsEnraged       = false;

	// HP 비율로 다음 페이즈 전환 확인
	void CheckAndTransitionPhase();

	// 페이즈 전환 시작 (AI 일시정지 + 델리게이트 브로드캐스트)
	virtual void StartPhaseTransition(int32 NewPhase);

// ─── 공격 패턴 ────────────────────────────────────────────────────────────
public:
	// 공격 패턴 선택 (가중치+쿨다운 기반)
	const FOHSMBossAttackPattern* SelectBossAttackPattern();

	// 패턴 쿨다운 등록
	void RegisterPatternCooldown(FName PatternID, float Cooldown);

	// 데미지 배율 (광폭화 시 변경)
	UFUNCTION(BlueprintPure, Category = "Boss|Combat")
	float GetDamageMultiplier() const { return bIsEnraged ? EnrageDamageMultiplier : 1.0f; }

	// 타겟 방향으로 즉시 회전
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void FaceTarget();

	// ── ① 기본 펀치 ─────────────────────────────────────────────────────
	// 주먹 히트박스 활성/비활성 (AnimNotify 호출)
	virtual void SetAttackCollisionEnabled(bool bEnable) override;

	// ─── BasicAttack 근접 히트박스 ──────────────────────────────────────
public:
	// 주먹 히트박스 (hand_r 소켓 부착)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Combat")
	TObjectPtr<class USphereComponent> PunchHitbox;

	// 스윙 중 히트 액터 목록 (중복 타격 방지)
	TSet<TObjectPtr<AActor>> HitActorsThisSwing;

	// ── ② 바위 던지기 ────────────────────────────────────────────────────
	// 바위 투사체 발사 (AnimNotify_SpawnProjectile 호출)
	virtual void LaunchSkillProjectile() override;

	// ── ③ 점프 공격 ──────────────────────────────────────────────────────
	// 점프 공격 시작 (AnimNotify_BossJump 호출)
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void TriggerJumpAttack();

	// ── ④ 에너지 방출 (주변 AoE) ─────────────────────────────────────────
	// 에너지 방출 AoE (AnimNotify_BossAreaAttack 호출)
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	virtual void TriggerAreaAttack();

	// ── ⑤ 전방 지면 강타 ─────────────────────────────────────────────────
	// 차징 이펙트 시작 (AnimNotify_BossGroundSlamCharge 호출)
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	virtual void TriggerGroundSlamCharge();

	// 지면 강타 발동 (AnimNotify_BossGroundSlam 호출)
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	virtual void TriggerGroundSlam();

protected:
	// 1페이즈 공격 패턴 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	TObjectPtr<UDataTable> Phase1AttackTable;

	// 2페이즈 공격 패턴 테이블 (없으면 Phase1 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	TObjectPtr<UDataTable> Phase2AttackTable;

	// 3페이즈 공격 패턴 테이블 (없으면 Phase2→Phase1 폴백)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	TObjectPtr<UDataTable> Phase3AttackTable;

	// 현재 페이즈 테이블 반환
	UDataTable* GetCurrentPhaseAttackTable() const;

	// 패턴 쿨다운 만료 여부 확인
	bool IsPatternOnCooldown(FName PatternID) const;

	// 직전 선택 패턴 포인터
	const FOHSMBossAttackPattern* ActivePattern = nullptr;

	// ─── ② 바위 던지기 설정 ─────────────────────────────────────────────
	// 바위 투사체 클래스 (BP 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	TSubclassOf<class AOHSMBossRockProjectile> RockProjectileClass;

	// 발사 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	FName RockSpawnSocket = TEXT("Hand_R");

	// ─── ③ 점프 공격 상태 ───────────────────────────────────────────────
	// 도약 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	TObjectPtr<class UNiagaraSystem> JumpStartEffect;

	// 착지 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	TObjectPtr<class UNiagaraSystem> JumpLandEffect;

	// 착지 카메라 셰이크
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	TSubclassOf<class UCameraShakeBase> JumpLandCameraShake;

	bool    bIsJumpAttacking     = false;
	FVector JumpAttackLandTarget = FVector::ZeroVector;
	FVector JumpAttackStartPos   = FVector::ZeroVector;
	float   JumpAttackElapsed    = 0.f;
	float   JumpAttackDuration   = 0.8f;

	// 점프 호 높이 (cm)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Jump")
	float JumpArcHeight = 500.f;

	// ─── ④ 에너지 방출 이펙트 ────────────────────────────────────────────
	// 에너지 방출 이펙트 (0→AreaRadius 스케일 보간)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Area")
	TObjectPtr<class UNiagaraSystem> AreaAttackEffect;

	// 이펙트 확장 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Area",
		meta = (ClampMin = "0.1"))
	float AreaExpandDuration = 0.8f;

	TObjectPtr<class UNiagaraComponent> ActiveAreaEffectComp;
	bool  bIsAreaExpanding    = false;
	float AreaExpandElapsed   = 0.f;
	float AreaExpandMaxRadius = 0.f;
	float AreaExpandDamage    = 0.f;

	// ─── 공격 범위 인디케이터 (Decal) ───────────────────────────────────────
	// 원형 범위 표시 데칼 머티리얼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Indicator")
	TObjectPtr<class UMaterialInterface> AttackIndicatorMaterial;

	// 부채꼴 범위 표시 데칼 머티리얼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Indicator")
	TObjectPtr<class UMaterialInterface> SectorIndicatorMaterial;

	// ─── ⑤ 전방 지면 강타 (GroundSlam) ─────────────────────────────────────
	// 차징 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|GroundSlam")
	TObjectPtr<class UNiagaraSystem> SlamChargeEffect;

	// 차징 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|GroundSlam")
	FName SlamChargeSocket = TEXT("SlamEffectSocket");

	// 충격파 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|GroundSlam")
	TObjectPtr<class UNiagaraSystem> SlamWaveEffect;

public:
	bool IsJumpAttacking() const { return bIsJumpAttacking; }

protected:
	virtual void Tick(float DeltaSeconds) override;

	// AoE 데미지 적용 (ActivePattern 기반)
	void ApplyBossAoeDamage(FVector Origin, float Radius, float Damage);

private:
	// 패턴 쿨다운 만료 시각 맵
	TMap<FName, float> PatternCooldownMap;

	// 파동 타이머 핸들 배열
	TArray<FTimerHandle> SlamWaveTimers;

	// 차징 일시정지 타이머
	FTimerHandle SlamChargeTimer;

	// 활성 차징 이펙트 컴포넌트
	TObjectPtr<class UNiagaraComponent> ActiveSlamChargeComp;

	// 활성 인디케이터 데칼
	TObjectPtr<class UDecalComponent> JumpIndicatorDecal;
	TObjectPtr<class UDecalComponent> SlamIndicatorDecal;
	TObjectPtr<class UDecalComponent> AreaIndicatorDecal;
	TObjectPtr<class UDecalComponent> ProjectileIndicatorDecal;

	// 데칼 제거 헬퍼
	void ClearDecal(TObjectPtr<class UDecalComponent>& Decal);

	// 원형 데칼 스폰
	void SpawnCircleDecal(TObjectPtr<class UDecalComponent>& OutDecal,
		FVector WorldLocation, float Radius);

	// 부채꼴 데칼 스폰 (GroundSlam)
	void SpawnSectorDecal(TObjectPtr<class UDecalComponent>& OutDecal,
		FVector FootLocation, float Length, float HalfAngleDeg);

	// 차징 완료 후 몽타주 재개
	void ResumeSlamMontage();

	// 단일 파동 실행
	void ExecuteSlamWave(int32 WaveIndex, float SlamLength,
		float HalfAngleDeg, float Damage, int32 TotalWaves);

	// 주먹 히트박스 오버랩 콜백
	UFUNCTION()
	void OnPunchHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// 페이즈 전환 몽타주 종료 콜백
	UFUNCTION()
	void OnPhaseChangeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// HealthComponent->OnHealthChanged 바인딩 — 페이즈 전환 체크
	UFUNCTION()
	void OnBossHealthChanged(float CurrentHealth, float MaxHealth, float Damage, AActor* DamageCauser);

	// 몽타주 종료 → FinishPhaseTransition 1초 지연 타이머
	FTimerHandle PhaseFinishTimerHandle;

public:
	// 사용 가능한 패턴 여부 확인 (BTTask_BossChase 참조)
	bool HasAnyAvailablePattern() const;
};
