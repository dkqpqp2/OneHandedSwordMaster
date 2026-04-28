// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMCharacterHUDInterface.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMCharacterInterface.h"
#include "OHSMPlayerCharacter.generated.h"

class UOHSMPlayerStatComponent;

UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMPlayerCharacter : public ACharacter, public IOHSMCharacterInterface, public IOHSMCharacterHUDInterface
{
	GENERATED_BODY()

public:
	AOHSMPlayerCharacter();
	
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;
	
	// 공격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InventoryAction;

	// 장비창
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> EquipmentAction;

	// 제작창
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> CraftAction;

	// 상호작용 (E키)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InteractAction;

	// 스킬창 토글
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SkillPanelAction;

	// 자동이동 (B키)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AutoMoveAction;

	// 포션 퀵슬롯 단축키 (1, 2, 3, 4)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuickPotionSlot4Action;

	// 스킬 퀵슬롯 단축키 (F1, F2, F3, F4)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SkillSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SkillSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SkillSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SkillSlot4Action;
	
// ============= 무기 시스템 =============
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AOHSMWeaponBase> CurrentWeapon;

	/** 일반 공격 트레일 — 무기 소켓에 붙어있고 평소엔 비활성 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Trail", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraComponent> WeaponTrailComponent;

	/** BP_PlayerCharacter Class Defaults 에서 일반 공격 트레일 에셋 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trail")
	TObjectPtr<class UNiagaraSystem> WeaponTrailSystem;

	/** 스킬 전용 트레일 — 스킬마다 다른 에셋을 AnimNotifyState에서 지정 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Trail", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraComponent> SkillTrailComponent;

public:
	/** AnimNotifyState_WeaponTrail 에서 호출 (일반 공격) */
	void ActivateWeaponTrail();
	void DeactivateWeaponTrail();

	/** AnimNotifyState_SkillTrail 에서 호출 — 스킬마다 다른 에셋 전달 */
	void ActivateSkillTrail(class UNiagaraSystem* TrailSystem);
	void DeactivateSkillTrail();

	/** OHSMSlashSkill 에서 호출 — 투사체 스폰 정보를 미리 저장 */
	void PrepareProjectileSpawn(TSubclassOf<class AOHSMSlashProjectile> InProjectileClass, float InDamage, float InForwardOffset);

	/** AnimNotify_SpawnProjectile 에서 호출 — 실제 투사체 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void SpawnPreparedProjectile();

private:
	TSubclassOf<class AOHSMSlashProjectile> PendingProjectileClass;
	float PendingProjectileDamage = 0.f;
	float PendingProjectileForwardOffset = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AOHSMWeaponBase> WeaponClass;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(class AOHSMWeaponBase* Weapon);
	
	AOHSMWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
	
// 공격 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMCombatComponent> CombatComponent;
	
// 인벤토리 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMInventoryComponent> InventoryComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UOHSMInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

// 퀵슬롯 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMQuickSlotComponent> QuickSlotComponent;

public:
	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	UOHSMQuickSlotComponent* GetQuickSlotComponent() const { return QuickSlotComponent; }

// 장비 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMEquipmentComponent> EquipmentComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Equipment")
	class UOHSMEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

// 제작 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMCraftComponent> CraftComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Craft")
	class UOHSMCraftComponent* GetCraftComponent() const { return CraftComponent; }

// 스킬 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMSkillComponent> SkillComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Skill")
	class UOHSMSkillComponent* GetSkillComponent() const { return SkillComponent; }

// 퀘스트 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMQuestComponent> QuestComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Quest")
	class UOHSMQuestComponent* GetQuestComponent() const { return QuestComponent; }

// 퀘스트 네비게이션 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMQuestNavigationComponent> QuestNavComponent;


protected:
	void ToggleInventory();

// ─── 사망 / 리스폰 ────────────────────────────────────────────────────────
public:
	/** 래그돌 해제 + 지정 위치로 이동 + HP 회복 */
	void Revive(FVector RespawnLocation, float HealPercent = 1.0f);

	/** 인벤토리에 부활 포션이 있는지 */
	bool HasRevivalPotion() const;

	/** 인벤토리에 있는 부활 포션 개수 */
	int32 GetRevivalPotionCount() const;

	/** 부활 포션 1개 소모. 성공 시 true */
	bool ConsumeRevivalPotion();

	bool bIsDead = false;

	/** 인벤토리에서 부활 포션으로 사용할 ItemID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	FName RevivalPotionID = TEXT("ResurrectionPotion");

// ─── 넉다운 (보스 점프 착지 등) ───────────────────────────────────────────
public:
	/**
	 * LaunchCharacter 기반 넉다운.
	 * LaunchVelocity : 날아갈 속도 벡터 (cm/s).  예) 방향 * 500 + Z * 800
	 * 착지 후 자동으로 일어나기 몽타주를 재생합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Knockdown")
	void TriggerKnockdown(FVector LaunchVelocity);

	/** 넉다운 or 일어나기 애니메이션 중이면 true */
	bool IsKnockedDown() const { return bIsKnockedDown; }

protected:
	/** 착지 감지 → 일어나기 시작 */
	virtual void Landed(const FHitResult& Hit) override;

	/** 착지 후 일어나기 몽타주 재생 + 입력 복구 */
	void DoGetUp();

	/** 앞으로 쓰러졌을 때 일어나기 몽타주 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Knockdown")
	TObjectPtr<UAnimMontage> GetUpFrontMontage;

	/** 뒤로 쓰러졌을 때 일어나기 몽타주 — 없으면 Front 로 대체 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Knockdown")
	TObjectPtr<UAnimMontage> GetUpBackMontage;

	bool         bIsKnockedDown     = false;
	FTimerHandle KnockdownGetUpTimer;   // 착지 딜레이 → DoGetUp
	FTimerHandle GetUpInputTimer;       // 일어나기 몽타주 → 입력 복구

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Attack();
	void SetDead();

	void UsePotionSlot1();
	void UsePotionSlot2();
	void UsePotionSlot3();
	void UsePotionSlot4();

	// 스킬 퀵슬롯 사용 (F1 ~ F4)
	void UseSkillSlot1();
	void UseSkillSlot2();
	void UseSkillSlot3();
	void UseSkillSlot4();

	/** 스킬 퀵슬롯 인덱스 (0~3) 의 스킬을 발동 */
	void UseSkillSlot(int32 SkillSlotIndex);

	void ToggleEquipment();
	void ToggleCraftPanel();
	void ToggleSkillPanel();
	void Interact();
	void AutoMove();

	UFUNCTION()
	void OnLevelUp(int32 NewLevel, int32 OldLevel);

	/** 스킬 습득 시 호출 — 콤보 해금 패시브 처리 */
	void OnSkillLearnedForCombo(FName SkillID);
	
public:
	TObjectPtr<class USpringArmComponent> GetCameraBoom() const { return CameraBoom; }
	TObjectPtr<class UCameraComponent> GetFollowCamera() const { return FollowCamera; }
	TObjectPtr<class UOHSMCombatComponent> GetCombatComponent() const { return CombatComponent; }
	
	UFUNCTION(BlueprintPure, Category = "Components")
	UOHSMPlayerStatComponent* GetStatComponent() const { return PlayerStat; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMPlayerStatComponent> PlayerStat;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOHSMWidgetComponent> HpBar;
	
	virtual void SetupCharacterWidget(class UOHSMUserWidget* InUserWidget) override;
	virtual void SetupHUDWidget(class UOHSMHUDWidget* InHUDWidget) override;
	
protected:
	/** 경험치 추가 (테스트용) */
	UFUNCTION(Exec)
	void AddExp(int32 Amount);
	
};
