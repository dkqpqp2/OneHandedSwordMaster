// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBossBase.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "OHSMBossRockProjectile.h"
#include "Engine/OverlapResult.h"
#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
AOHSMBossBase::AOHSMBossBase()
{
	// 보스 플래그 — EnemyBase의 HUD 보스 체력바 표시에 사용
	bIsBoss = true;
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BossMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonMinions/Characters/Minions/Prime_Helix/Meshes/Prime_Helix.Prime_Helix'"));
	if (BossMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(BossMeshRef.Object);
	}
	
	static ConstructorHelpers::FClassFinder<UAnimInstance> BossAnimInstanceClassRef(TEXT("/Script/Engine.AnimBlueprint'/Game/OneHandedSwordMaster/Animations/ABP_OHSMBoss.ABP_OHSMBoss_C'"));
	if (BossAnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(BossAnimInstanceClassRef.Class);
	}
	
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -220.0f),
		FRotator(0.0f, -90.0f, 0.0f));
	
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetCapsuleComponent()->InitCapsuleSize(150.0f, 220.0f);
	
	
}

void AOHSMBossBase::BeginPlay()
{
	Super::BeginPlay();
}

// ─── TakeDamage 오버라이드 ─────────────────────────────────────────────────

float AOHSMBossBase::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	// 사망 처리 중이거나 페이즈 전환 중이면 무시
	if (!bIsDead && !bIsPhaseChanging)
	{
		CheckAndTransitionPhase();
	}

	return ActualDamage;
}

// ─── 페이즈 시스템 ────────────────────────────────────────────────────────

void AOHSMBossBase::CheckAndTransitionPhase()
{
	if (!HealthComponent)
	{
		return;
	}

	const float HpPercent = HealthComponent->GetHealthPercent();

	// PhaseThresholds = {0.6f, 0.3f}
	// index 0 (60%): Phase 1 → 2
	// index 1 (30%): Phase 2 → 3
	int32 TargetPhase = 1;
	for (int32 i = PhaseThresholds.Num() - 1; i >= 0; --i)
	{
		if (HpPercent <= PhaseThresholds[i])
		{
			TargetPhase = i + 2; // index 0 → Phase 2, index 1 → Phase 3
			break;
		}
	}

	if (TargetPhase > CurrentPhase)
	{
		StartPhaseTransition(TargetPhase);
	}
}

void AOHSMBossBase::StartPhaseTransition(int32 NewPhase)
{
	bIsPhaseChanging = true;
	CurrentPhase     = NewPhase;

	// AI 일시 정지 (BTTask_BossPhaseChange가 처리 완료 후 Resume 호출)
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->PauseLogic(TEXT("BossPhaseChange"));
		}
	}

	// HUD 등 외부 구독자에게 페이즈 변경 알림
	OnPhaseChanged.Broadcast(NewPhase);

	UE_LOG(LogTemp, Log, TEXT("[Boss] 페이즈 %d 전환 시작"), NewPhase);
}

void AOHSMBossBase::FinishPhaseTransition()
{
	bIsPhaseChanging = false;

	// AI 재개
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("BossPhaseChange"));
		}
	}

	// Phase 3 광폭화 (최초 1회)
	if (CurrentPhase == 3 && !bIsEnraged)
	{
		bIsEnraged = true;

		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->MaxWalkSpeed *= EnrageSpeedMultiplier;
		}

		UE_LOG(LogTemp, Log, TEXT("[Boss] 광폭화! 이동 속도 x%.1f, 데미지 x%.1f"),
			EnrageSpeedMultiplier, EnrageDamageMultiplier);
	}

	UE_LOG(LogTemp, Log, TEXT("[Boss] 페이즈 %d 전환 완료"), CurrentPhase);
}

// ─── 공통 유틸 ────────────────────────────────────────────────────────────

void AOHSMBossBase::FaceTarget()
{
	if (!TargetActor)
	{
		return;
	}

	FVector Dir = TargetActor->GetActorLocation() - GetActorLocation();
	Dir.Z = 0.f; // 수직 방향 무시 (Y축 회전만)

	if (!Dir.IsNearlyZero())
	{
		SetActorRotation(Dir.Rotation());
	}
}

// ─── 공격 패턴 ────────────────────────────────────────────────────────────

const FOHSMBossAttackPattern* AOHSMBossBase::SelectBossAttackPattern()
{
	UDataTable* Table = GetCurrentPhaseAttackTable();
	if (!Table)
	{
		return nullptr;
	}

	TArray<FName> RowNames = Table->GetRowNames();

	// 타겟까지 거리 계산
	const float DistToTarget = TargetActor
		? FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation())
		: 0.f;

	// 현재 페이즈 & 쿨다운 통과 후보 수집
	TArray<TPair<FName, const FOHSMBossAttackPattern*>> Candidates;
	float TotalWeight = 0.f;

	for (const FName& RowName : RowNames)
	{
		const FOHSMBossAttackPattern* Pattern =
			Table->FindRow<FOHSMBossAttackPattern>(RowName, TEXT(""));

		if (!Pattern)
		{
			continue;
		}

		// Phase 0 = 전 페이즈, 또는 현재 페이즈와 일치
		if (Pattern->Phase != 0 && Pattern->Phase != CurrentPhase)
		{
			continue;
		}

		if (IsPatternOnCooldown(RowName))
		{
			continue;
		}

		// ── 사거리 기반 필터 ──────────────────────────────────────────────
		// AttackRange 필드의 의미가 패턴 타입에 따라 다름:
		//   근접 (Normal / Area / GroundSlam) : AttackRange = 최대 사거리
		//     → 플레이어가 AttackRange 보다 멀면 제외 (가까울 때만 사용)
		//   원거리 (Jump / Projectile)        : AttackRange = 최소 사거리
		//     → 플레이어가 AttackRange 보다 가까우면 제외 (멀 때만 사용)
		//   AttackRange == 0 : 거리 제한 없음
		const bool bIsRangedType =
			Pattern->AttackType == EBossAttackType::Jump       ||
			Pattern->AttackType == EBossAttackType::Projectile;

		if (Pattern->AttackRange > 0.f)
		{
			if (!bIsRangedType && DistToTarget > Pattern->AttackRange)
			{
				continue; // 근접 패턴: 최대 사거리 밖 → 제외
			}
			if (bIsRangedType && DistToTarget < Pattern->AttackRange)
			{
				continue; // 원거리 패턴: 최소 사거리 안 → 너무 가까움, 제외
			}
		}

		Candidates.Add({RowName, Pattern});
		TotalWeight += Pattern->Weight;
	}

	if (Candidates.IsEmpty())
	{
		return nullptr;
	}

	// 가중치 기반 랜덤 선택
	float Rand = FMath::FRandRange(0.f, TotalWeight);
	float Accumulated = 0.f;

	for (auto& [ID, Pattern] : Candidates)
	{
		Accumulated += Pattern->Weight;
		if (Rand <= Accumulated)
		{
			RegisterPatternCooldown(ID, Pattern->Cooldown);
			ActivePattern = Pattern;
			return Pattern;
		}
	}

	// 폴백: 마지막 후보
	RegisterPatternCooldown(Candidates.Last().Key, Candidates.Last().Value->Cooldown);
	ActivePattern = Candidates.Last().Value;
	return ActivePattern;
}

void AOHSMBossBase::RegisterPatternCooldown(FName PatternID, float Cooldown)
{
	if (UWorld* World = GetWorld())
	{
		PatternCooldownMap.Add(PatternID, World->GetTimeSeconds() + Cooldown);
	}
}

bool AOHSMBossBase::IsPatternOnCooldown(FName PatternID) const
{
	const float* ExpireTime = PatternCooldownMap.Find(PatternID);
	if (!ExpireTime)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return World && World->GetTimeSeconds() < *ExpireTime;
}

UDataTable* AOHSMBossBase::GetCurrentPhaseAttackTable() const
{
	switch (CurrentPhase)
	{
	case 3:
		if (Phase3AttackTable) return Phase3AttackTable;
		// fallthrough
	case 2:
		if (Phase2AttackTable) return Phase2AttackTable;
		// fallthrough
	default:
		return Phase1AttackTable;
	}
}

// ═══════════════════════════════════════════════════════════════════════════
// ② 바위 던지기
// ═══════════════════════════════════════════════════════════════════════════

void AOHSMBossBase::LaunchSkillProjectile()
{
	if (!RockProjectileClass || !TargetActor || !GetWorld())
	{
		return;
	}

	const FVector SpawnLoc   = GetMesh()->GetSocketLocation(RockSpawnSocket);
	const FVector TargetLoc  = TargetActor->GetActorLocation();

	const float GravityScale = ActivePattern ? ActivePattern->ProjectileGravityScale : 0.7f;
	const float Speed        = ActivePattern ? ActivePattern->ProjectileSpeed        : 1000.f;

	// ── 포물선 궤적 계산 ──────────────────────────────────────────────────
	// SuggestProjectileVelocity_CustomArc 로 실제 중력(스케일 보정)을 감안한
	// 발사 방향 벡터를 구한다. 실패 시 직선 조준으로 폴백.
	const float GravityZ = GetWorld()->GetGravityZ() * GravityScale; // 음수값

	FVector OutVelocity;
	const bool bSolved = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		GetWorld(),
		OutVelocity,
		SpawnLoc,
		TargetLoc,
		GravityZ,  // 실제 사용 중력 반영
		0.5f       // ArcParam: 0=저탄도, 1=고탄도 (0.5 = 중간 포물선)
	);

	const FVector Direction = bSolved
		? OutVelocity.GetSafeNormal()
		: (TargetLoc - SpawnLoc).GetSafeNormal(); // 폴백

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner      = this;
	SpawnParams.Instigator = this;

	AOHSMBossRockProjectile* Rock = GetWorld()->SpawnActor<AOHSMBossRockProjectile>(
		RockProjectileClass,
		SpawnLoc,
		Direction.Rotation(),
		SpawnParams
	);

	if (Rock && ActivePattern)
	{
		const float Damage       = ActivePattern->Damage * GetDamageMultiplier();
		const float ImpactRadius = ActivePattern->ProjectileImpactRadius;

		Rock->InitProjectile(this, Direction, Damage, Speed, GravityScale, ImpactRadius);
	}

	UE_LOG(LogTemp, Log, TEXT("[Boss] 바위 발사 — 방향(%s), 속도%.0f, ArcSolved:%s"),
		*Direction.ToString(), Speed, bSolved ? TEXT("OK") : TEXT("Fallback"));
}

// ═══════════════════════════════════════════════════════════════════════════
// ③ 점프 공격
// ═══════════════════════════════════════════════════════════════════════════

void AOHSMBossBase::TriggerJumpAttack()
{
	if (!TargetActor)
	{
		return;
	}

	// 시작 위치 저장 + 이동 파라미터 초기화
	bIsJumpAttacking   = true;
	JumpAttackStartPos = GetActorLocation();

	// 플레이어 위치에서 보스 방향으로 150cm 앞에 착지 → 캡슐 끼임 방지
	const FVector ToTarget  = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	JumpAttackLandTarget    = TargetActor->GetActorLocation() - ToTarget * 150.f;
	JumpAttackElapsed    = 0.f;
	JumpAttackDuration   = ActivePattern ? ActivePattern->JumpDuration : 0.8f;

	// 이동 컴포넌트 완전 비활성화 — Tick의 SetActorLocation이 완전 제어
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	// 타겟 방향으로 회전
	FaceTarget();

	// 도약 시작 이펙트
	if (JumpStartEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), JumpStartEffect, GetActorLocation(), FRotator::ZeroRotator);
	}

	UE_LOG(LogTemp, Log, TEXT("[Boss] 점프 공격 시작 → 체공 %.1fs"), JumpAttackDuration);
}

void AOHSMBossBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ── ④ 에너지 방출 이펙트 확장 ──────────────────────────────────────
	if (bIsAreaExpanding)
	{
		AreaExpandElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(AreaExpandElapsed / AreaExpandDuration, 0.f, 1.f);

		// Niagara User.Radius 파라미터로 반경 제어 → 완전한 원형 유지
		if (IsValid(ActiveAreaEffectComp))
		{
			const float CurRadius = FMath::Lerp(0.f, AreaExpandMaxRadius, Alpha);
			ActiveAreaEffectComp->SetFloatParameter(TEXT("User.Radius"), CurRadius);
		}

		if (Alpha >= 1.f)
		{
			bIsAreaExpanding = false;

			// 최대 범위 도달 → 데미지 적용
			ApplyBossAoeDamage(GetActorLocation(), AreaExpandMaxRadius, AreaExpandDamage);

			// 컴포넌트 완전 제거 (Deactivate만 하면 기존 파티클이 화면에 남음)
			if (IsValid(ActiveAreaEffectComp))
			{
				ActiveAreaEffectComp->DeactivateImmediate();
				ActiveAreaEffectComp->DestroyComponent();
				ActiveAreaEffectComp = nullptr;
			}

			UE_LOG(LogTemp, Log, TEXT("[Boss] 에너지 방출 완료 — 반경 %.0f, 데미지 %.0f"),
				AreaExpandMaxRadius, AreaExpandDamage);
		}
	}

	// ── ③ 점프 공격 이동 ───────────────────────────────────────────────
	if (!bIsJumpAttacking)
	{
		return;
	}

	JumpAttackElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(JumpAttackElapsed / JumpAttackDuration, 0.f, 1.f);

	// 선형 보간 위치 + 사인 곡선으로 포물선 호 적용
	FVector NewPos    = FMath::Lerp(JumpAttackStartPos, JumpAttackLandTarget, Alpha);
	NewPos.Z         += JumpArcHeight * FMath::Sin(Alpha * PI);

	SetActorLocation(NewPos, false, nullptr, ETeleportType::TeleportPhysics);

	// 착지 완료
	if (Alpha >= 1.f)
	{
		bIsJumpAttacking = false;

		// 이동 모드 복구
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		const float Radius = ActivePattern ? ActivePattern->JumpLandingRadius : 350.f;
		const float JumpDmg = ActivePattern
			? ActivePattern->Damage * GetDamageMultiplier() : 30.f;

		// 착지 AoE 데미지 (ActivePattern 기반 — PerformAttack은 보스에서 사용 불가)
		ApplyBossAoeDamage(GetActorLocation(), Radius, JumpDmg);

		// ── 착지 반경 내 플레이어 래그돌 ──────────────────────────────────
		if (AOHSMPlayerCharacter* PlayerChar = Cast<AOHSMPlayerCharacter>(TargetActor))
		{
			const float DistToPlayer =
				FVector::Dist2D(GetActorLocation(), PlayerChar->GetActorLocation());

			if (DistToPlayer <= Radius)
			{
				const FVector HorizDir =
					(PlayerChar->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
				// 수평(뒤로 밀림)과 수직(위로 튀어오름)을 분리해서 직관적으로 조정
				const FVector LaunchVelocity =
					HorizDir * 400.f            // 수평: 멀리 날아가는 거리 조정
					+ FVector(0.f, 0.f, 700.f); // 수직: 높이 올라가는 정도 조정

				PlayerChar->TriggerKnockdown(LaunchVelocity);
			}
		}

		// 착지 이펙트
		if (JumpLandEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), JumpLandEffect, GetActorLocation(), FRotator::ZeroRotator);
		}

		// 카메라 셰이크
		if (JumpLandCameraShake)
		{
			UGameplayStatics::PlayWorldCameraShake(
				GetWorld(), JumpLandCameraShake, GetActorLocation(),
				0.f, Radius * 2.f);
		}

		UE_LOG(LogTemp, Log, TEXT("[Boss] 착지 완료 — AoE 반경 %.0f"), Radius);
	}
}

// ═══════════════════════════════════════════════════════════════════════════
// ④ 에너지 방출 (주변 AoE)
// ═══════════════════════════════════════════════════════════════════════════

void AOHSMBossBase::TriggerAreaAttack()
{
	if (!ActivePattern)
	{
		return;
	}

	// 이전 이펙트가 남아 있으면 정리
	if (IsValid(ActiveAreaEffectComp))
	{
		ActiveAreaEffectComp->DeactivateImmediate();
		ActiveAreaEffectComp = nullptr;
	}

	AreaExpandMaxRadius = ActivePattern->AreaRadius;          // 최대 반경 (예: 500)
	AreaExpandDamage    = ActivePattern->Damage * GetDamageMultiplier();
	AreaExpandElapsed   = 0.f;
	bIsAreaExpanding    = true;

	// 이펙트 스폰 — User.Radius = 0 에서 시작, Tick에서 AreaExpandMaxRadius 까지 증가
	if (AreaAttackEffect)
	{
		ActiveAreaEffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AreaAttackEffect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(1.f),               // 스케일은 1 고정 (크기는 User.Radius로 제어)
			false,                      // bAutoDestroy = false (Tick에서 직접 제거)
			true
		);

		if (IsValid(ActiveAreaEffectComp))
		{
			// 시작 반경 0
			ActiveAreaEffectComp->SetFloatParameter(TEXT("User.Radius"), 0.f);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Boss] 에너지 방출 시작 — 목표 반경 %.0f, %.1fs 확장"),
		AreaExpandMaxRadius, AreaExpandDuration);
}

// ═══════════════════════════════════════════════════════════════════════════
// ⑤ 전방 지면 강타
// ═══════════════════════════════════════════════════════════════════════════

void AOHSMBossBase::TriggerGroundSlam()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 패턴에서 범위·데미지 읽기
	float SlamLength = 500.f;
	float SlamWidth  = 250.f;
	float Damage     = 40.f;

	if (ActivePattern)
	{
		SlamLength = ActivePattern->ForwardSlamLength;
		SlamWidth  = ActivePattern->ForwardSlamWidth;
		Damage     = ActivePattern->Damage * GetDamageMultiplier();
	}

	// 박스 중심 = 보스 위치 + 전방으로 절반 길이
	const FVector Origin    = GetActorLocation();
	const FVector Forward   = GetActorForwardVector();
	const FVector BoxCenter = Origin + Forward * (SlamLength * 0.5f);
	const FQuat   BoxRot    = GetActorQuat();
	const FVector BoxHalf   = FVector(SlamLength * 0.5f, SlamWidth * 0.5f, 100.f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(
		Overlaps,
		BoxCenter,
		BoxRot,
		ECC_Pawn,
		FCollisionShape::MakeBox(BoxHalf),
		QueryParams
	);

	// 중복 방지용 Set
	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActors.Contains(HitActor))
		{
			continue;
		}
		HitActors.Add(HitActor);

		UGameplayStatics::ApplyDamage(
			HitActor,
			Damage,
			GetController(),
			this,
			nullptr
		);
	}

	// 디버그 박스 (개발 중 시각화)
#if WITH_EDITOR
	DrawDebugBox(World, BoxCenter, BoxHalf, BoxRot,
		FColor::Orange, false, 1.5f, 0, 3.f);
#endif

	UE_LOG(LogTemp, Log, TEXT("[Boss] 전방 강타 — 길이 %.0f, 폭 %.0f, 맞은 수 %d"),
		SlamLength, SlamWidth, HitActors.Num());
}

// ═══════════════════════════════════════════════════════════════════════════
// AoE 데미지 공통 헬퍼
// ═══════════════════════════════════════════════════════════════════════════

void AOHSMBossBase::ApplyBossAoeDamage(FVector Origin, float Radius, float Damage)
{
	UWorld* World = GetWorld();
	if (!World || Radius <= 0.f || Damage <= 0.f)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	World->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActors.Contains(HitActor))
		{
			continue;
		}
		// "Player" 태그가 있는 액터에만 데미지
		if (!HitActor->ActorHasTag(TEXT("Player")))
		{
			continue;
		}

		HitActors.Add(HitActor);
		UGameplayStatics::ApplyDamage(HitActor, Damage, GetController(), this, nullptr);

		UE_LOG(LogTemp, Log, TEXT("[Boss] AoE 데미지 %.0f → %s"),
			Damage, *HitActor->GetName());
	}

#if WITH_EDITOR
	DrawDebugSphere(World, Origin, Radius, 16, FColor::Orange, false, 1.5f, 0, 2.f);
#endif
}

// ═══════════════════════════════════════════════════════════════════════════
// 패턴 가용 여부 확인 (선택하지 않음 — BTTask_BossChase 에서 호출)
// ═══════════════════════════════════════════════════════════════════════════

bool AOHSMBossBase::HasAnyAvailablePattern() const
{
	UDataTable* Table = GetCurrentPhaseAttackTable();
	if (!Table)
	{
		return false;
	}

	const float DistToTarget = TargetActor
		? FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation())
		: 0.f;

	for (const FName& RowName : Table->GetRowNames())
	{
		const FOHSMBossAttackPattern* Pattern =
			Table->FindRow<FOHSMBossAttackPattern>(RowName, TEXT(""));

		if (!Pattern) continue;

		// 페이즈 필터
		if (Pattern->Phase != 0 && Pattern->Phase != CurrentPhase) continue;

		// 쿨다운 필터
		if (IsPatternOnCooldown(RowName)) continue;

		// 거리 필터 (SelectBossAttackPattern 과 동일 로직)
		const bool bIsRangedType =
			Pattern->AttackType == EBossAttackType::Jump ||
			Pattern->AttackType == EBossAttackType::Projectile;

		if (Pattern->AttackRange > 0.f)
		{
			if (!bIsRangedType && DistToTarget > Pattern->AttackRange) continue;
			if (bIsRangedType  && DistToTarget < Pattern->AttackRange) continue;
		}

		return true; // 사용 가능한 패턴 하나 이상 존재
	}

	return false;
}
