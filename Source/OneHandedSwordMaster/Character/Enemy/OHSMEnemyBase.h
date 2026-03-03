// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"
#include "NiagaraSystem.h"
#include "OHSMEnemyBase.generated.h"

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Run			UMETA(DisplayName = "Run"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Skill		UMETA(DisplayName = "Skill"),
	Dead		UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EEnemyPersonality : uint8
{
	Aggressive	UMETA(DisplayName = "Aggressive"),	// 즉시 공격
	Passive		UMETA(DisplayName = "Passive")		// 맞아야 공격
};

UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AOHSMEnemyBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UOHSMHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> DetectionSphere;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Personality")
	EEnemyPersonality Personality = EEnemyPersonality::Aggressive;
	
	/** 감지 범위 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Detection", meta = (ClampMin = "100"))
	float DetectionRange = 500.0f;
	
	/** 공격 범위 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat", meta = (ClampMin = "50"))
	float AttackRange = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat", meta = (ClampMin = "50"))
	float LeashRange = 1000.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "AI|Leash")
	FVector HomeLocation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Leash")
	bool bHealOnLeash = true;

	/** 이동 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Movement")
	float WalkSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Movement")
	float RunSpeed = 400.0f;
	
public:
	// 스킬 범위 공격 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	TObjectPtr<UNiagaraSystem> SkillHitEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Effect")
	TObjectPtr<UNiagaraSystem> AttackHitEffect;
	
protected:
	/** 공격 패턴 DataTable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UDataTable> AttackPatternTable;

	/** 현재 AI 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	EEnemyAIState CurrentState = EEnemyAIState::Idle;
	
	FEnemyAttackPattern* CurrentAttackPattern = nullptr;

	/** 타겟 (플레이어) */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> TargetActor;

	/** 현재 공격 중 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	/** 마지막 스킬 사용 시간 */
	UPROPERTY()
	TMap<FName, float> SkillCooldowns;
	
public:
	/** 타겟 설정 (AI Controller가 호출) */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTarget(AActor* NewTarget);

	/** 공격 실행 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack(bool bIsAreaAttack = false, float Radius = 0.0f, AActor* HitTargetActor = nullptr);

	/** 공격 가능 범위인지 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInAttackRange() const;

	/** AI 상태 변경 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAIState(EEnemyAIState NewState);

	/** 타겟 가져오기 */
	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetTarget() const { return TargetActor; }
	
	UFUNCTION(BlueprintPure, Category = "AI|Leash")
	bool IsOutOfLeashRange() const;
	
	UFUNCTION(BlueprintCallable, Category = "AI|Leash")
	void ReturnToHome();
	
	UFUNCTION(BlueprintCallable, Category = "AI|Leash")
	FVector GetHomeLocation() const { return HomeLocation; }

public:
	float GetHalfHeight() const
	{
		return GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	
public:
	virtual void ChangeAIAnimType(uint8 AnimType);
	
public:
	/** 공격 패턴 선택 */
	struct FEnemyAttackPattern* SelectAttackPattern();

protected:
	/** 감지 범위 오버랩 */
	UFUNCTION()
	void OnDetectionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
						   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						   bool bFromSweep, const FHitResult& SweepResult);

	/** 죽음 처리 */
	UFUNCTION()
	void OnDeath(AActor* Killer);

	/** 데미지 받았을 때 (Passive용) */
	UFUNCTION()
	void OnDamaged(float CurrentHealth, float MaxHealth, float Damage, AActor* DamageCauser);
};
