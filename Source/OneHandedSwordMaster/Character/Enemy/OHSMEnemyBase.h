// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"
#include "NiagaraSystem.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMEnemyBase.generated.h"

/** 스포너가 사망 감지를 위해 구독하는 델리게이트 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyDeathNotify, AOHSMEnemyBase* /*DeadEnemy*/);

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Spawning	UMETA(DisplayName = "Spawning"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Run			UMETA(DisplayName = "Run"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Skill		UMETA(DisplayName = "Skill"),
	Dead		UMETA(DisplayName = "Dead"),
	PhaseChange	UMETA(DisplayName = "PhaseChange")	// 보스 페이즈 전환 연출 중
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

	// 머리 위 체력바 위젯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UOHSMWidgetComponent> EnemyHpBar;

	// 이 거리 이내의 적 체력바만 표시 (cm)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (ClampMin = "100"))
	float HpBarDisplayRange = 1000.0f;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Personality")
	EEnemyPersonality Personality = EEnemyPersonality::Aggressive;
	
	// 감지 범위 (cm)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Detection", meta = (ClampMin = "100"))
	float DetectionRange = 500.0f;

	// 공격 범위 (cm)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat", meta = (ClampMin = "50"))
	float AttackRange = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat", meta = (ClampMin = "50"))
	float LeashRange = 1000.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "AI|Leash")
	FVector HomeLocation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Leash")
	bool bHealOnLeash = true;
	
	UPROPERTY()
	bool bIsDead = false;

	/** 사망 시 브로드캐스트 — 스포너가 구독하여 리스폰 타이머 시작 */
	FOnEnemyDeathNotify OnEnemyDeathNotify;

	// ─── 보스 설정 ─────────────────────────────────────────────
	/** 보스 여부 - true이면 화면 상단에 보스 체력바 표시 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	bool bIsBoss = false;

	/** 보스 이름 (비어있으면 Actor 이름 사용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	FText BossDisplayName;

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
	// 타겟 설정
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTarget(AActor* NewTarget);

	// 공격 실행
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack(bool bIsAreaAttack = false, float Radius = 0.0f, AActor* HitTargetActor = nullptr);

	// 공격 범위 내 여부
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInAttackRange() const;

	// AI 상태 변경
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAIState(EEnemyAIState NewState);

	// 타겟 반환
	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetTarget() const { return TargetActor; }

	// 사망 여부
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsDead() const { return bIsDead; }

	// 체력 컴포넌트 반환
	UFUNCTION(BlueprintPure, Category = "Components")
	class UOHSMHealthComponent* GetHealthComponent() const { return HealthComponent; }

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

	/** AnimNotify OnCollision / OffCollision 에서 호출 — 자식이 override */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void SetAttackCollisionEnabled(bool bEnable) {}

	/** AnimNotify SpawnProjectile 에서 호출 — 자식이 override */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void LaunchSkillProjectile() {}
	
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
	
	
// 스폰 애니메이션
protected:
	/** 스폰 시 ABP Spawning Pose 로 재생할 애니메이션 시퀀스 — 길이만큼 AI 일시정지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UAnimSequence> SpawnAnimSequence;

	/** SpawnAnimSequence 재생 완료 후 호출 — AI 재개 및 Idle 전환 */
	UFUNCTION()
	void OnSpawnEnded();

// 경험치 / 퀘스트
protected:
	/** 처치 시 플레이어에게 지급할 경험치 — Blueprint에서 몬스터마다 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ClampMin = "0"))
	int32 ExpReward = 50;

	/** 퀘스트 Kill 목표의 TargetID 와 매칭할 고유 식별자 (예: "Slime", "EnemyRed") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName EnemyID;

// ItemDrop
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop")
	TArray<FDropItemData> DropItem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop")
	TObjectPtr<UDataTable> DropItemTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop")
	TMap<FName, TSubclassOf<class AOHSMPickupItem>> PickupItemClasses;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop")
	TSubclassOf<class AOHSMPickupItem> PickupItemClass;
protected:
	UFUNCTION(BlueprintCallable, Category = "Item Drop")
	void DropItems();
};
