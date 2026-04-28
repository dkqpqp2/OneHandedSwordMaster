// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPlayerCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "EngineUtils.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "OHSMPlayerController.h"
#include "OneHandedSwordMaster/Character/UI/OHSMWidgetComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMCombatComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMEquipmentComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMCraftComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestNavigationComponent.h"
#include "Components/StaticMeshComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Weapon/OHSMWeaponBase.h"
#include "OneHandedSwordMaster/Character/UI/OHSMHpBar.h"
#include "OneHandedSwordMaster/Character/UI/OHSMHUDWidget.h"
#include "OneHandedSwordMaster/Character/UI/OHSMManaBar.h"
#include "OneHandedSwordMaster/Character/UI/OHSMExpBar.h"
#include "OneHandedSwordMaster/Character/Skills/OHSMSlashProjectile.h"


// Sets default values
AOHSMPlayerCharacter::AOHSMPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick         = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // 래그돌 중에만 켜짐

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 700.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetRelativeRotation(FRotator(0.0f, -20.0f, 0.0f));

	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -100.0f),
		FRotator(0.0f, -90.0f, 0.0f)
		);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(FName("CharacterMesh"));

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Player"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple'"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/OneHandedSwordMaster/Animations/ABP_OHSMPlayer.ABP_OHSMPlayer_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
	}

	// 공격 컴포넌트 추가
	CombatComponent = CreateDefaultSubobject<UOHSMCombatComponent>(TEXT("CombatComponent"));

	// Stat Component
	PlayerStat = CreateDefaultSubobject<UOHSMPlayerStatComponent>(TEXT("PlayerStat"));

	// Widget Component
	HpBar = CreateDefaultSubobject<UOHSMWidgetComponent>(TEXT("Widget"));
	HpBar->SetupAttachment(GetMesh());
	HpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	static ConstructorHelpers::FClassFinder<UUserWidget> HpBarWidgetRef(TEXT("/Game/OneHandedSwordMaster/UI/WBP_HpBar.WBP_HpBar_C"));
	if (HpBarWidgetRef.Class)
	{
		HpBar->SetWidgetClass(HpBarWidgetRef.Class);
		HpBar->SetWidgetSpace(EWidgetSpace::Screen);
		HpBar->SetDrawSize(FVector2D(150.0f, 15.0f));
		HpBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	InventoryComponent  = CreateDefaultSubobject<UOHSMInventoryComponent>(TEXT("InventoryComponent"));
	QuickSlotComponent  = CreateDefaultSubobject<UOHSMQuickSlotComponent>(TEXT("QuickSlotComponent"));
	EquipmentComponent  = CreateDefaultSubobject<UOHSMEquipmentComponent>(TEXT("EquipmentComponent"));
	CraftComponent      = CreateDefaultSubobject<UOHSMCraftComponent>(TEXT("CraftComponent"));
	SkillComponent      = CreateDefaultSubobject<UOHSMSkillComponent>(TEXT("SkillComponent"));
	QuestComponent      = CreateDefaultSubobject<UOHSMQuestComponent>(TEXT("QuestComponent"));
	QuestNavComponent   = CreateDefaultSubobject<UOHSMQuestNavigationComponent>(TEXT("QuestNavComponent"));

	// 일반 공격 트레일
	WeaponTrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponTrailComponent"));
	WeaponTrailComponent->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));
	WeaponTrailComponent->SetAutoActivate(false);

	// 스킬 전용 트레일 (스킬마다 다른 에셋을 런타임에 교체)
	SkillTrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SkillTrailComponent"));
	SkillTrailComponent->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));
	SkillTrailComponent->SetAutoActivate(false);
}

void AOHSMPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PlayerStat->OnHpZero.AddUObject(this, &AOHSMPlayerCharacter::SetDead);

	// 레벨업 시 스킬 포인트 지급
	PlayerStat->OnLevelUp.AddDynamic(this, &AOHSMPlayerCharacter::OnLevelUp);

	// 스킬 습득 시 콤보 해금 처리
	SkillComponent->OnSkillLearned.AddUObject(this, &AOHSMPlayerCharacter::OnSkillLearnedForCombo);
}

// Called when the game starts or when spawned
void AOHSMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		CurrentWeapon = GetWorld()->SpawnActor<AOHSMWeaponBase>(WeaponClass, SpawnParams);

		if (CurrentWeapon)
		{
			CurrentWeapon->EquipToCharacter(this);
			// 장착된 무기가 없을 때는 메시를 비워둠 (장비창에서 장착 시 교체)
			CurrentWeapon->GetWeaponMesh()->SetStaticMesh(nullptr);
		}
	}

	// CurrentWeapon 스폰 이후에 기본 장착 아이템 적용 (무기 메시 교체 가능하도록)
	if (EquipmentComponent)
	{
		EquipmentComponent->InitializeDefaultEquipment();
	}

	// 레벨 1 시작 시 초기 스킬포인트 3개 지급
	if (SkillComponent)
	{
		SkillComponent->AddSkillPoints(3);
	}
}


void AOHSMPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// LaunchCharacter 방식은 CharacterMovement 가 캡슐을 직접 움직이므로 Tick 처리 불필요
}

// Called to bind functionality to input
void AOHSMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input Component로 캐스팅
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOHSMPlayerCharacter::Move);

		// 시점 이동
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOHSMPlayerCharacter::Look);

		// 점프
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// 공격
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::Attack);

		// 인벤토리
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Completed, this, &AOHSMPlayerCharacter::ToggleInventory);

		// 장비창
		if (EquipmentAction) EnhancedInputComponent->BindAction(EquipmentAction, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::ToggleEquipment);

		// 제작창
		if (CraftAction) EnhancedInputComponent->BindAction(CraftAction, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::ToggleCraftPanel);

		// 스킬창
		if (SkillPanelAction) EnhancedInputComponent->BindAction(SkillPanelAction, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::ToggleSkillPanel);

		// 상호작용 (E키)
		if (InteractAction) EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::Interact);

		// 자동이동 (B키)
		if (AutoMoveAction) EnhancedInputComponent->BindAction(AutoMoveAction, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::AutoMove);

		// 포션 퀵슬롯 (1, 2, 3, 4)
		if (QuickPotionSlot1Action) EnhancedInputComponent->BindAction(QuickPotionSlot1Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UsePotionSlot1);
		if (QuickPotionSlot2Action) EnhancedInputComponent->BindAction(QuickPotionSlot2Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UsePotionSlot2);
		if (QuickPotionSlot3Action) EnhancedInputComponent->BindAction(QuickPotionSlot3Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UsePotionSlot3);
		if (QuickPotionSlot4Action) EnhancedInputComponent->BindAction(QuickPotionSlot4Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UsePotionSlot4);

		// 스킬 퀵슬롯 (F1, F2, F3, F4)
		if (SkillSlot1Action) EnhancedInputComponent->BindAction(SkillSlot1Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UseSkillSlot1);
		if (SkillSlot2Action) EnhancedInputComponent->BindAction(SkillSlot2Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UseSkillSlot2);
		if (SkillSlot3Action) EnhancedInputComponent->BindAction(SkillSlot3Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UseSkillSlot3);
		if (SkillSlot4Action) EnhancedInputComponent->BindAction(SkillSlot4Action, ETriggerEvent::Started, this, &AOHSMPlayerCharacter::UseSkillSlot4);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

float AOHSMPlayerCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	PlayerStat->ApplyDamage(Damage);

	return Damage;
}

void AOHSMPlayerCharacter::EquipWeapon(class AOHSMWeaponBase* Weapon)
{
	if (!Weapon)
	{
		return;
	}
	CurrentWeapon = Weapon;
	Weapon->EquipToCharacter(this);
}

void AOHSMPlayerCharacter::ToggleInventory()
{
	AOHSMPlayerController* PlayerController = Cast<AOHSMPlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}
	PlayerController->ToggleInventory();
}

void AOHSMPlayerCharacter::ToggleEquipment()
{
	AOHSMPlayerController* PlayerController = Cast<AOHSMPlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}
	PlayerController->ToggleEquipment();
}

void AOHSMPlayerCharacter::ToggleCraftPanel()
{
	AOHSMPlayerController* PlayerController = Cast<AOHSMPlayerController>(GetController());
	if (!PlayerController) return;
	PlayerController->ToggleCraftPanel();
}

void AOHSMPlayerCharacter::ToggleSkillPanel()
{
	AOHSMPlayerController* PlayerController = Cast<AOHSMPlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}
	PlayerController->ToggleSkillPanel();
}

void AOHSMPlayerCharacter::OnLevelUp(int32 NewLevel, int32 OldLevel)
{
	if (!SkillComponent)
	{
		return;
	}

	SkillComponent->AddSkillPoints(3);
}

void AOHSMPlayerCharacter::OnSkillLearnedForCombo(FName SkillID)
{
	if (!CombatComponent || !SkillComponent)
	{
		return;
	}

	const FOHSMSkillData* Data = SkillComponent->GetSkillData(SkillID);
	if (!Data || Data->SkillType != ESkillType::Passive)
	{
		return;
	}

	// ComboCountUnlock > 0 인 패시브만 처리
	if (Data->ComboCountUnlock > 0)
	{
		CombatComponent->SetMaxComboCount(Data->ComboCountUnlock);
		UE_LOG(LogTemp, Log, TEXT("[Combo] 스킬 '%s' 습득 → 최대 콤보 %d타 해금"),
			*SkillID.ToString(), Data->ComboCountUnlock);
	}
}

void AOHSMPlayerCharacter::Interact()
{
	AOHSMPlayerController* PlayerController = Cast<AOHSMPlayerController>(GetController());
	if (!PlayerController) return;
	PlayerController->TryInteract();
}

void AOHSMPlayerCharacter::AutoMove()
{
	AOHSMPlayerController* PlayerController = Cast<AOHSMPlayerController>(GetController());
	if (!PlayerController) return;
	PlayerController->TriggerAutoMove();
}

void AOHSMPlayerCharacter::Move(const FInputActionValue& Value)
{
	// 입력 값 (Vector2D)
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 컨트롤러의 회전에서 Yaw만 가져오기
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 전방 방향 벡터
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// 우측 방향 벡터
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 이동 입력 추가
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AOHSMPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller == nullptr)
	{
		return;
	}

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AOHSMPlayerCharacter::Attack()
{
	APlayerController* PC = GetController<APlayerController>();
	if (PC && PC->bShowMouseCursor)
	{
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->PerformBasicAttack();
	}
}

void AOHSMPlayerCharacter::SetDead()
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		// 래그돌 활성화
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));

		// 캡슐 충돌 비활성화
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		if (Capsule)
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		// 이동 비활성화
		UCharacterMovementComponent* Movement = GetCharacterMovement();
		if (Movement)
		{
			Movement->DisableMovement();
			Movement->StopMovementImmediately();
		}

		HpBar->SetHiddenInGame(true);
	}

	// 입력 비활성화
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// 이 플레이어를 타겟으로 하는 모든 적 타겟 해제 → Idle 복귀
	for (TActorIterator<AOHSMEnemyBase> It(GetWorld()); It; ++It)
	{
		AOHSMEnemyBase* Enemy = *It;
		if (!IsValid(Enemy) || Enemy->IsDead())
		{
			continue;
		}
		if (Enemy->GetTarget() == this)
		{
			Enemy->SetTarget(nullptr);
			Enemy->SetAIState(EEnemyAIState::Idle);
		}
	}

	// 컨트롤러에 사망 통보 → 사망 화면 표시
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(GetController()))
	{
		PC->HandlePlayerDeath(GetRevivalPotionCount());
	}
}

bool AOHSMPlayerCharacter::HasRevivalPotion() const
{
	return GetRevivalPotionCount() > 0;
}

int32 AOHSMPlayerCharacter::GetRevivalPotionCount() const
{
	if (!InventoryComponent)
	{
		return 0;
	}
	return InventoryComponent->GetItemCount(RevivalPotionID);
}

bool AOHSMPlayerCharacter::ConsumeRevivalPotion()
{
	if (!InventoryComponent)
	{
		return false;
	}
	return InventoryComponent->RemoveItem(RevivalPotionID, 1) > 0;
}

void AOHSMPlayerCharacter::Revive(FVector RespawnLocation, float HealPercent)
{
	bIsDead = false;

	// 래그돌 해제 + 메시 재부착
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
		MeshComp->AttachToComponent(
			GetCapsuleComponent(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);
		MeshComp->SetRelativeLocationAndRotation(
			FVector(0.0f, 0.0f, -100.0f),
			FRotator(0.0f, -90.0f, 0.0f)
		);
	}

	// 캡슐 콜리전 복구
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// 이동 복구
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement)
	{
		Movement->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	// 리스폰 위치로 텔레포트
	SetActorLocation(RespawnLocation, false, nullptr, ETeleportType::TeleportPhysics);

	// HP 회복
	if (PlayerStat)
	{
		PlayerStat->Heal(PlayerStat->GetMaxHp() * HealPercent);
	}

	// HP 바 표시
	if (HpBar)
	{
		HpBar->SetHiddenInGame(false);
	}

	// 입력 복구
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		EnableInput(PC);
	}
}

void AOHSMPlayerCharacter::SetupCharacterWidget(class UOHSMUserWidget* InUserWidget)
{
	UOHSMHpBar* HpBarWidget = Cast<UOHSMHpBar>(InUserWidget);
	if (HpBarWidget)
	{
		float MaxHp = PlayerStat->GetMaxHp();
		float CurrentHp = PlayerStat->GetCurrentHp();

		HpBarWidget->SetMaxHp(MaxHp);
		HpBarWidget->UpdateHpBar(CurrentHp, MaxHp);
		PlayerStat->OnHpChanged.AddUObject(HpBarWidget, &UOHSMHpBar::UpdateHpBar);
	}
	else if (UOHSMManaBar* ManaBarWidget = Cast<UOHSMManaBar>(InUserWidget))
	{
		float MaxMana = PlayerStat->GetMaxMana();
		float CurrentMana = PlayerStat->GetCurrentMana();

		ManaBarWidget->SetMaxMana(MaxMana);
		ManaBarWidget->UpdateManaBar(CurrentMana, MaxMana);

		PlayerStat->OnManaChanged.AddUObject(ManaBarWidget, &UOHSMManaBar::UpdateManaBar);
	}
	else if (UOHSMExpBar* ExpBarWidget = Cast<UOHSMExpBar>(InUserWidget))
	{
		int32 CurrentExp = 0;
		int32 RequiredExp = 100;

		ExpBarWidget->UpdateExpBar(CurrentExp, RequiredExp);

		PlayerStat->OnExpChanged.AddDynamic(ExpBarWidget, &UOHSMExpBar::UpdateExpBar);
	}
}

void AOHSMPlayerCharacter::SetupHUDWidget(class UOHSMHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		InHUDWidget->UpdateHp(PlayerStat->GetCurrentHp(), PlayerStat->GetMaxHp());
		InHUDWidget->UpdateMana(PlayerStat->GetCurrentMana(), PlayerStat->GetMaxMana());
		InHUDWidget->OnExpChanged(0, PlayerStat->GetCurrentHp());
	}
}

void AOHSMPlayerCharacter::UsePotionSlot1() { if (QuickSlotComponent) QuickSlotComponent->UsePotionSlot(0); }
void AOHSMPlayerCharacter::UsePotionSlot2() { if (QuickSlotComponent) QuickSlotComponent->UsePotionSlot(1); }
void AOHSMPlayerCharacter::UsePotionSlot3() { if (QuickSlotComponent) QuickSlotComponent->UsePotionSlot(2); }
void AOHSMPlayerCharacter::UsePotionSlot4() { if (QuickSlotComponent) QuickSlotComponent->UsePotionSlot(3); }

void AOHSMPlayerCharacter::UseSkillSlot1() { UseSkillSlot(0); }
void AOHSMPlayerCharacter::UseSkillSlot2() { UseSkillSlot(1); }
void AOHSMPlayerCharacter::UseSkillSlot3() { UseSkillSlot(2); }
void AOHSMPlayerCharacter::UseSkillSlot4() { UseSkillSlot(3); }

void AOHSMPlayerCharacter::UseSkillSlot(int32 SkillSlotIndex)
{
	if (!SkillComponent || !QuickSlotComponent)
	{
		return;
	}

	// 스킬 슬롯은 QuickSlotComponent 인덱스 4~7 사용
	const FName SkillID = QuickSlotComponent->GetSlotItemID(
		UOHSMQuickSlotComponent::PotionSlotCount + SkillSlotIndex);

	if (SkillID.IsNone())
	{
		return;
	}

	SkillComponent->ActivateSkill(SkillID);
}

void AOHSMPlayerCharacter::AddExp(int32 Amount)
{
	if (PlayerStat)
	{
		PlayerStat->AddExperience(Amount);
	}
}

// ─── 넉다운 (LaunchCharacter 기반) ───────────────────────────────────────

void AOHSMPlayerCharacter::TriggerKnockdown(FVector LaunchVelocity)
{
	// 이미 사망 or 넉다운 중이면 무시
	if (bIsDead || bIsKnockedDown)
	{
		return;
	}
	bIsKnockedDown = true;

	GetWorldTimerManager().ClearTimer(KnockdownGetUpTimer);
	GetWorldTimerManager().ClearTimer(GetUpInputTimer);

	// 입력 비활성화 (날아가는 동안 조작 불가)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// CharacterMovement가 캡슐 자체를 날려줌 → 카메라가 자연스럽게 따라옴
	// bXYOverride=true, bZOverride=true : 현재 속도를 완전히 교체
	LaunchCharacter(LaunchVelocity, true, true);

	UE_LOG(LogTemp, Log, TEXT("[Player] 넉다운 — 발사 속도 X:%.0f Y:%.0f Z:%.0f"),
		LaunchVelocity.X, LaunchVelocity.Y, LaunchVelocity.Z);
}

void AOHSMPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (!bIsKnockedDown)
	{
		return;
	}

	// 착지 직후 0.2초 대기 후 일어나기 (즉시 일어나면 어색)
	GetWorldTimerManager().SetTimer(
		KnockdownGetUpTimer,
		this,
		&AOHSMPlayerCharacter::DoGetUp,
		0.2f,
		false
	);
}

void AOHSMPlayerCharacter::DoGetUp()
{
	if (bIsDead)
	{
		bIsKnockedDown = false;
		return;
	}

	// 일어나기 몽타주 — Back 없으면 Front 로 대체
	UAnimMontage* GetUpMontage = GetUpFrontMontage ? GetUpFrontMontage : GetUpBackMontage;
	float MontageDuration = 0.f;
	if (GetUpMontage)
	{
		MontageDuration = PlayAnimMontage(GetUpMontage);
	}

	// 몽타주 끝난 뒤 입력 복구
	auto FinishKnockdown = [this]()
	{
		bIsKnockedDown = false;
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			EnableInput(PC);
		}
		UE_LOG(LogTemp, Log, TEXT("[Player] 넉다운 완전 회복"));
	};

	if (MontageDuration > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			GetUpInputTimer,
			FTimerDelegate::CreateLambda(FinishKnockdown),
			MontageDuration,
			false
		);
	}
	else
	{
		FinishKnockdown();
	}
}

void AOHSMPlayerCharacter::ActivateWeaponTrail()
{
	if (!IsValid(WeaponTrailComponent))
	{
		return;
	}

	if (IsValid(WeaponTrailSystem) && WeaponTrailComponent->GetAsset() != WeaponTrailSystem)
	{
		WeaponTrailComponent->SetAsset(WeaponTrailSystem);
	}

	WeaponTrailComponent->Activate(true);
}

void AOHSMPlayerCharacter::DeactivateWeaponTrail()
{
	if (IsValid(WeaponTrailComponent))
	{
		WeaponTrailComponent->Deactivate();
	}
}

void AOHSMPlayerCharacter::ActivateSkillTrail(UNiagaraSystem* TrailSystem)
{
	if (!IsValid(SkillTrailComponent) || !IsValid(TrailSystem))
	{
		return;
	}

	// 스킬마다 다른 에셋으로 교체 후 활성화
	SkillTrailComponent->SetAsset(TrailSystem);
	SkillTrailComponent->Activate(true);
}

void AOHSMPlayerCharacter::DeactivateSkillTrail()
{
	if (IsValid(SkillTrailComponent))
	{
		SkillTrailComponent->Deactivate();
	}
}

void AOHSMPlayerCharacter::PrepareProjectileSpawn(TSubclassOf<AOHSMSlashProjectile> InProjectileClass, float InDamage, float InForwardOffset)
{
	PendingProjectileClass       = InProjectileClass;
	PendingProjectileDamage      = InDamage;
	PendingProjectileForwardOffset = InForwardOffset;
}

void AOHSMPlayerCharacter::SpawnPreparedProjectile()
{
	if (!PendingProjectileClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector  Forward  = GetActorForwardVector();
	const FVector  Origin   = GetActorLocation();
	const FVector  SpawnPos = Origin + Forward * PendingProjectileForwardOffset;
	const FRotator SpawnRot = Forward.Rotation();

	FActorSpawnParameters Params;
	Params.Owner     = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AOHSMSlashProjectile* Projectile =
		World->SpawnActor<AOHSMSlashProjectile>(PendingProjectileClass, SpawnPos, SpawnRot, Params);

	if (IsValid(Projectile))
	{
		Projectile->InitProjectile(this, PendingProjectileDamage);
	}

	// 사용 후 초기화
	PendingProjectileClass = nullptr;
	PendingProjectileDamage = 0.f;
}
