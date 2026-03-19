// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPlayerCharacter.h"

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
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Weapon/OHSMWeaponBase.h"
#include "OneHandedSwordMaster/Character/UI/OHSMHpBar.h"
#include "OneHandedSwordMaster/Character/UI/OHSMHUDWidget.h"
#include "OneHandedSwordMaster/Character/UI/OHSMManaBar.h"
#include "OneHandedSwordMaster/Character/UI/OHSMExpBar.h"


// Sets default values
AOHSMPlayerCharacter::AOHSMPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
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
	
	InventoryComponent = CreateDefaultSubobject<UOHSMInventoryComponent>(TEXT("InventoryComponent"));
}

void AOHSMPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	PlayerStat->OnHpZero.AddUObject(this, &AOHSMPlayerCharacter::SetDead);
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
		}
	}
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
	// 입력 값 (Vector2D)
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Yaw (좌우) 회전 추가
		AddControllerYawInput(LookAxisVector.X);
        
		// Pitch (상하) 회전 추가
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AOHSMPlayerCharacter::Attack()
{
	if (CombatComponent)
	{
		CombatComponent->PerformBasicAttack();
	}
}

void AOHSMPlayerCharacter::SetDead()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
	    // 물리 시뮬레이션 활성화
    	MeshComp->SetSimulatePhysics(true);
    	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
        
    	// 캡슐 충돌 비활성화
    	UCapsuleComponent* Capsule = GetCapsuleComponent();
    	if (Capsule)
    	{
    		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    	}
        
    	// CharacterMovement 비활성화
    	UCharacterMovementComponent* Movement = GetCharacterMovement();
    	if (Movement)
    	{
    		Movement->DisableMovement();
    		Movement->StopMovementImmediately();
    	}
    	
    	HpBar->SetHiddenInGame(true);
    }
}

void AOHSMPlayerCharacter:: SetupCharacterWidget(class UOHSMUserWidget* InUserWidget)
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

void AOHSMPlayerCharacter::AddExp(int32 Amount)
{
	if (PlayerStat)
	{
		PlayerStat->AddExperience(Amount);
	}
}

