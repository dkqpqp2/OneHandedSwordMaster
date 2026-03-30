// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMTargetingComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

UOHSMTargetingComponent::UOHSMTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOHSMTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsTargeting)
	{
		return;
	}

	ValidateTarget();

	if (bIsTargeting)
	{
		UpdateRotations(DeltaTime);
	}
}

// =========================================================
// 락온 토글
// =========================================================
void UOHSMTargetingComponent::ToggleLockOn()
{
	if (bIsTargeting)
	{
		SetTarget(nullptr);
		return;
	}

	TArray<AActor*> Enemies = GetEnemiesInRange();
	AActor* BestTarget = FindBestTarget(Enemies);
	SetTarget(BestTarget);
}

// =========================================================
// 마우스 X 누적 → 임계값 초과 시 방향 전환
// =========================================================
void UOHSMTargetingComponent::AddMouseDeltaX(float DeltaX)
{
	if (!bIsTargeting)
	{
		AccumulatedMouseX = 0.0f;
		return;
	}

	AccumulatedMouseX += DeltaX;

	if (AccumulatedMouseX > SwitchThreshold)
	{
		SwitchTargetByDirection(true);   // 오른쪽
		AccumulatedMouseX = 0.0f;
	}
	else if (AccumulatedMouseX < -SwitchThreshold)
	{
		SwitchTargetByDirection(false);  // 왼쪽
		AccumulatedMouseX = 0.0f;
	}
}

// =========================================================
// 화면 기준 오른쪽 / 왼쪽 적으로 전환
// =========================================================
void UOHSMTargetingComponent::SwitchTargetByDirection(bool bRight)
{
	if (!CurrentTarget)
	{
		return;
	}

	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	// 현재 타겟의 스크린 X 좌표
	FVector2D CurrentScreenPos;
	PC->ProjectWorldLocationToScreen(CurrentTarget->GetActorLocation(), CurrentScreenPos);

	TArray<AActor*> Enemies = GetEnemiesInRange();

	AActor* BestTarget = nullptr;
	float BestDist = FLT_MAX;

	for (AActor* Enemy : Enemies)
	{
		if (Enemy == CurrentTarget)
		{
			continue;
		}

		FVector2D EnemyScreenPos;
		PC->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), EnemyScreenPos);

		// 원하는 방향에 있는 적만 후보로
		float ScreenDeltaX = EnemyScreenPos.X - CurrentScreenPos.X;
		bool bIsRight = ScreenDeltaX > 0.0f;

		if (bRight != bIsRight)
		{
			continue;
		}

		// 현재 타겟과 화면 거리가 가장 가까운 적 선택
		float Dist = FVector2D::Distance(EnemyScreenPos, CurrentScreenPos);
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestTarget = Enemy;
		}
	}

	if (BestTarget)
	{
		SetTarget(BestTarget);
	}
}

// =========================================================
// 조건 1: 거리 + 화면 내에 있는 살아있는 적 수집
// =========================================================
TArray<AActor*> UOHSMTargetingComponent::GetEnemiesInRange() const
{
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOHSMEnemyBase::StaticClass(), AllEnemies);

	APlayerController* PC = GetPlayerController();
	AActor* Owner = GetOwner();
	TArray<AActor*> Result;

	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

	for (AActor* Enemy : AllEnemies)
	{
		AOHSMEnemyBase* EnemyBase = Cast<AOHSMEnemyBase>(Enemy);
		if (!EnemyBase || EnemyBase->IsDead())
		{
			continue;
		}

		// 조건 1: 거리 범위
		float Distance = FVector::Dist(Owner->GetActorLocation(), Enemy->GetActorLocation());
		if (Distance > MaxTargetingRange)
		{
			continue;
		}

		// 조건 2: 화면 안에 있는지
		FVector2D ScreenPos;
		bool bOnScreen = PC->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenPos);
		if (!bOnScreen)
		{
			continue;
		}

		// 화면 밖 좌표 체크
		if (ScreenPos.X < 0 || ScreenPos.X > ViewportSizeX ||
			ScreenPos.Y < 0 || ScreenPos.Y > ViewportSizeY)
		{
			continue;
		}

		Result.Add(Enemy);
	}

	return Result;
}

// =========================================================
// 조건 3: 거리(70%) + 화면 중앙 거리(30%) 가중치 스코어로 최적 타겟 선정
// =========================================================
AActor* UOHSMTargetingComponent::FindBestTarget(const TArray<AActor*>& Candidates) const
{
	if (Candidates.IsEmpty())
	{
		return nullptr;
	}

	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	FVector2D ScreenCenter(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

	AActor* Owner = GetOwner();
	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	for (AActor* Candidate : Candidates)
	{
		// ── 거리 점수 (가까울수록 1에 수렴) ──
		float Distance = FVector::Dist(Owner->GetActorLocation(), Candidate->GetActorLocation());
		float DistanceScore = 1.0f / (Distance + 1.0f);

		// ── 화면 중앙 거리 점수 (중앙에 가까울수록 1에 수렴) ──
		FVector2D ScreenPos;
		PC->ProjectWorldLocationToScreen(Candidate->GetActorLocation(), ScreenPos);
		float ScreenCenterDist = FVector2D::Distance(ScreenPos, ScreenCenter);
		float ScreenScore = 1.0f / (ScreenCenterDist + 1.0f);

		// ── 최종 가중치 점수: 거리 70% + 화면 중앙 30% ──
		float FinalScore = (DistanceScore * 0.7f) + (ScreenScore * 0.3f);

		if (FinalScore > BestScore)
		{
			BestScore = FinalScore;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void UOHSMTargetingComponent::SetTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
	bIsTargeting = (NewTarget != nullptr);

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		// 락온 중: 이동 방향으로 자동 회전 OFF, 컨트롤러 Yaw ON
		OwnerChar->bUseControllerRotationYaw   = bIsTargeting;
		OwnerChar->GetCharacterMovement()->bOrientRotationToMovement = !bIsTargeting;
	}

	OnTargetChanged.Broadcast(CurrentTarget);
}

void UOHSMTargetingComponent::ValidateTarget()
{
	if (!CurrentTarget)
	{
		SetTarget(nullptr);
		return;
	}

	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(CurrentTarget);
	if (Enemy && Enemy->IsDead())
	{
		SetTarget(nullptr);
		return;
	}

	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > AutoReleaseRange)
	{
		SetTarget(nullptr);
	}
}

// =========================================================
// 카메라 / 캐릭터 회전 업데이트 (Tick)
// =========================================================
void UOHSMTargetingComponent::UpdateRotations(float DeltaTime)
{
	if (!CurrentTarget)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	APlayerController* PC = GetPlayerController();
	if (!OwnerChar || !PC)
	{
		return;
	}

	FVector OwnerLocation  = OwnerChar->GetActorLocation();
	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector ToTarget       = (TargetLocation - OwnerLocation).GetSafeNormal();
	FRotator TargetRot     = ToTarget.Rotation();

	// ── 카메라 보간 회전 (Pitch는 30%만 반영해 자연스럽게) ──
	FRotator NewControlRot = FMath::RInterpTo(
		PC->GetControlRotation(),
		FRotator(TargetRot.Pitch * 0.3f, TargetRot.Yaw, 0.0f),
		DeltaTime,
		CameraInterpSpeed
	);
	PC->SetControlRotation(NewControlRot);

	// ── 캐릭터 Yaw 보간 회전 ──
	FRotator NewActorRot = FMath::RInterpTo(
		OwnerChar->GetActorRotation(),
		FRotator(0.0f, TargetRot.Yaw, 0.0f),
		DeltaTime,
		CharacterInterpSpeed
	);
	OwnerChar->SetActorRotation(NewActorRot);
}

// =========================================================
// 헬퍼: PlayerController 가져오기
// =========================================================
APlayerController* UOHSMTargetingComponent::GetPlayerController() const
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return nullptr;
	}
	return Cast<APlayerController>(OwnerChar->GetController());
}
