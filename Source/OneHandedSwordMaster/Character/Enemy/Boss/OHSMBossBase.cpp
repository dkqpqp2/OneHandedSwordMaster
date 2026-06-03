// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBossBase.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "OHSMBossRockProjectile.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Core/OHSMBossMapGameMode.h"
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

	// ─── 주먹 히트박스 (hand_rSocket 에 부착) ────────────────────────────
	PunchHitbox = CreateDefaultSubobject<USphereComponent>(TEXT("PunchHitbox"));
	PunchHitbox->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	PunchHitbox->SetSphereRadius(70.f);
	PunchHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 기본 비활성화
	PunchHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	PunchHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	PunchHitbox->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // Player 채널
}

void AOHSMBossBase::BeginPlay()
{
	Super::BeginPlay();

	// 주먹 히트박스 오버랩 바인딩
	if (PunchHitbox)
	{
		PunchHitbox->OnComponentBeginOverlap.AddDynamic(
			this, &AOHSMBossBase::OnPunchHitboxOverlap);
	}

	// BossMapGameMode에 자신의 사망 이벤트 등록
	// (보스는 동적 스폰이므로 GameMode::BeginPlay 이후에 스폰됨 — 여기서 등록해야 함)
	if (AOHSMBossMapGameMode* BossGM = Cast<AOHSMBossMapGameMode>(GetWorld()->GetAuthGameMode()))
	{
		OnEnemyDeathNotify.AddUObject(BossGM, &AOHSMBossMapGameMode::OnBossDied);
	}

	// HealthComponent->OnHealthChanged 에 페이즈 전환 체크 바인딩
	// 플레이어 무기/스킬은 HealthComponent->TakeDamage() 를 직접 호출하므로
	// Actor::TakeDamage() 오버라이드가 아닌 이 델리게이트에서 체크해야 함
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AOHSMBossBase::OnBossHealthChanged);
	}
}

// HealthComponent->OnHealthChanged 콜백 — 실제 페이즈 전환 진입점
void AOHSMBossBase::OnBossHealthChanged(float CurrentHealth, float MaxHealth, float Damage, AActor* DamageCauser)
{
	// Damage <= 0 은 회복이므로 무시, 사망 중·페이즈 전환 중도 무시
	if (Damage <= 0.f || bIsDead || bIsPhaseChanging) return;

	CheckAndTransitionPhase();
}

void AOHSMBossBase::CheckAndTransitionPhase()
{
	if (!HealthComponent)
	{
		return;
	}

	const float HpPercent = HealthComponent->GetHealthPercent();
	
	int32 TargetPhase = 1;
	for (int32 i = PhaseThresholds.Num() - 1; i >= 0; --i)
	{
		if (HpPercent <= PhaseThresholds[i])
		{
			TargetPhase = i + 2;
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
	CurrentPhase = NewPhase;
	
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->PauseLogic(TEXT("BossPhaseChange"));
		}
	}
	
	if (PhaseChangeMontage)
	{
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->OnMontageEnded.AddDynamic(
				this, &AOHSMBossBase::OnPhaseChangeMontageEnded);
		}
		PlayAnimMontage(PhaseChangeMontage);
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			PhaseFinishTimerHandle,
			this,
			&AOHSMBossBase::FinishPhaseTransition,
			1.0f,
			false);
	}
	
	if (PhaseChangeEffect)
	{
		for (const FName& SocketName : { FName(TEXT("LeftHandSocket")), FName(TEXT("RightHandSocket")) })
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				PhaseChangeEffect,
				GetMesh(),
				SocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}
	}
	
	OnPhaseChanged.Broadcast(NewPhase);

	UE_LOG(LogTemp, Log, TEXT("[Boss] 페이즈 %d 전환 시작"), NewPhase);
}

// 페이즈 전환 몽타주 종료 → 1초 대기 후 FinishPhaseTransition
void AOHSMBossBase::OnPhaseChangeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != PhaseChangeMontage) return;

	// 콜백 즉시 해제 (중복 호출 방지)
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->OnMontageEnded.RemoveDynamic(
			this, &AOHSMBossBase::OnPhaseChangeMontageEnded);
	}

	// 몽타주 종료 후 1초 대기 → AI 재개 + 스킬 사용 허용
	GetWorldTimerManager().SetTimer(
		PhaseFinishTimerHandle,
		this,
		&AOHSMBossBase::FinishPhaseTransition,
		1.0f,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("[Boss] 페이즈 전환 몽타주 종료 — 1초 후 전투 재개"));
}

// 페이즈 전환 완료
void AOHSMBossBase::FinishPhaseTransition()
{
	bIsPhaseChanging = false; // 전환 완료

	// AI 재개
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("BossPhaseChange"));
		}
	}

	// Phase 2 광폭화 (최초 1회) — 이동속도 + 데미지 배율 적용
	if (CurrentPhase == 2 && !bIsEnraged)
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

// ─── ① 기본 펀치 히트박스 ─────────────────────────────────────────────────

void AOHSMBossBase::SetAttackCollisionEnabled(bool bEnable)
{
	if (!PunchHitbox) return;

	if (bEnable)
	{
		// 새 스윙 시작 — 이전 피격 목록 초기화
		HitActorsThisSwing.Empty();
		PunchHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		PunchHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AOHSMBossBase::OnPunchHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this) return;
	if (!OtherActor->ActorHasTag(TEXT("Player"))) return;

	// 한 스윙에서 같은 대상 중복 타격 방지
	if (HitActorsThisSwing.Contains(OtherActor)) return;
	HitActorsThisSwing.Add(OtherActor);

	// ActivePattern 의 Damage 사용 (광폭화 배율 반영)
	float Damage = 45.f; // 패턴 없을 때 기본값
	if (ActivePattern)
	{
		Damage = ActivePattern->Damage * GetDamageMultiplier();
	}

	OtherActor->TakeDamage(Damage, FDamageEvent(), GetController(), this);
}

// ─── 공통 유틸 ────────────────────────────────────────────────────────────

// 타겟 방향으로 회전
void AOHSMBossBase::FaceTarget()
{
	if (!TargetActor)
	{
		return;
	}

	FVector Dir = TargetActor->GetActorLocation() - GetActorLocation();
	Dir.Z = 0.f; // 수평 회전만

	if (!Dir.IsNearlyZero())
	{
		SetActorRotation(Dir.Rotation()); // 즉시 회전
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
				continue; // 근접: 최대 사거리 밖 → 제외
			}
			if (bIsRangedType && DistToTarget < Pattern->AttackRange)
			{
				continue; // 원거리: 최소 사거리 미달 → 너무 가까움, 제외
			}
		}

		// 원거리 패턴 상한 거리 (MaxAttackRange > 0 일 때만 적용)
		if (bIsRangedType && Pattern->MaxAttackRange > 0.f &&
			DistToTarget > Pattern->MaxAttackRange)
		{
			continue; // 원거리: 최대 사거리 초과 → 너무 멀어서 걸어서 접근
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

	// ── 포물선 궤적 계산 (직접 물리 공식) ───────────────────────────────
	// 수평 거리 / 속도 ≈ 비행 시간 T
	// 중력 보정: Vz = ΔHeight/T + 0.5 * g * T
	// → 속도가 빠를수록 더 직선에 가깝게, 느릴수록 포물선이 높아짐.
	// 외부 라이브러리 없이 항상 DataTable 속도가 그대로 반영됨.

	const float G         = FMath::Abs(GetWorld()->GetGravityZ()) * GravityScale; // 양수
	const float HorizDist = FVector::Dist2D(SpawnLoc, TargetLoc);
	const float HeightDiff = TargetLoc.Z - SpawnLoc.Z;

	// 수평 방향 (Z 무시)
	const FVector HorizDir = HorizDist > 1.f
		? (FVector(TargetLoc.X, TargetLoc.Y, SpawnLoc.Z) - SpawnLoc).GetSafeNormal()
		: GetActorForwardVector();

	// 근사 비행 시간 (수평 속도 ≈ Speed 가정)
	const float T  = HorizDist / FMath::Max(Speed, 1.f);

	// 타겟 높이까지 도달하는 데 필요한 수직 속도
	const float Vz = HeightDiff / T + 0.5f * G * T;

	// 수평 Speed + 보정된 Vz 로 발사 방향 결정, 속도는 DataTable 그대로
	const FVector RawVelocity = HorizDir * Speed + FVector(0.f, 0.f, Vz);
	const FVector LaunchDir   = RawVelocity.GetSafeNormal();
	const float   LaunchSpeed = Speed;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner      = this;
	SpawnParams.Instigator = this;

	AOHSMBossRockProjectile* Rock = GetWorld()->SpawnActor<AOHSMBossRockProjectile>(
		RockProjectileClass,
		SpawnLoc,
		LaunchDir.Rotation(),
		SpawnParams
	);

	if (Rock && ActivePattern)
	{
		const float Damage       = ActivePattern->Damage * GetDamageMultiplier();
		const float ImpactRadius = ActivePattern->ProjectileImpactRadius;

		Rock->InitProjectile(this, LaunchDir, Damage, LaunchSpeed, GravityScale, ImpactRadius);

		// 착탄 예상 위치에 원형 데칼 인디케이터
		SpawnCircleDecal(ProjectileIndicatorDecal, TargetLoc, ImpactRadius);
		// 투사체 비행 시간 후 자동 제거
		const float FlightTime = HorizDist / FMath::Max(LaunchSpeed, 1.f);
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				ClearDecal(ProjectileIndicatorDecal);
			}),
			FlightTime + 0.1f, false);
	}

	UE_LOG(LogTemp, Log, TEXT("[Boss] 바위 발사 — 속도%.0f, Vz%.0f, 거리%.0f"),
		LaunchSpeed, Vz, HorizDist);
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

	// 착지 예상 위치에 원형 데칼 인디케이터
	const float JumpRadius = ActivePattern ? ActivePattern->JumpLandingRadius : 350.f;
	SpawnCircleDecal(JumpIndicatorDecal, JumpAttackLandTarget, JumpRadius);

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

			// 인디케이터 제거
			ClearDecal(AreaIndicatorDecal);

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

		// 착지 인디케이터 제거
		ClearDecal(JumpIndicatorDecal);

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

		// 착지 이펙트 — 발 위치(캡슐 하단)에서 스폰
		if (JumpLandEffect)
		{
			const FVector FootLocation = GetActorLocation()
				- FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), JumpLandEffect, FootLocation, FRotator::ZeroRotator);
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

	AreaExpandMaxRadius = ActivePattern->AreaRadius;
	AreaExpandDamage    = ActivePattern->Damage * GetDamageMultiplier();
	AreaExpandElapsed   = 0.f;
	bIsAreaExpanding    = true;

	// 에너지 방출 범위 데칼 인디케이터
	SpawnCircleDecal(AreaIndicatorDecal, GetActorLocation(), AreaExpandMaxRadius);

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
// ⑤ 전방 지면 강타 — 부채꼴 다중 파동 (펑 펑 펑)
// ═══════════════════════════════════════════════════════════════════════════

// ── 차징 (기모으기 프레임 노티파이) ─────────────────────────────────────
// 1) 차징 이펙트 재생  2) 몽타주 일시정지  3) 랜덤 타이머 → 재개
void AOHSMBossBase::TriggerGroundSlamCharge()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 차징 이펙트 — SlamEffectSocket 위치에서 스폰
	if (SlamChargeEffect)
	{
		// 소켓이 있으면 소켓 위치/회전 사용, 없으면 보스 위치로 폴백
		const bool   bHasSocket = GetMesh()->DoesSocketExist(SlamChargeSocket);
		const FVector SpawnLoc  = bHasSocket
			? GetMesh()->GetSocketLocation(SlamChargeSocket)
			: GetActorLocation();
		const FRotator SpawnRot = bHasSocket
			? GetMesh()->GetSocketRotation(SlamChargeSocket)
			: GetActorRotation();

		// 이전 차징 이펙트가 남아있으면 먼저 제거
		if (IsValid(ActiveSlamChargeComp))
		{
			ActiveSlamChargeComp->DeactivateImmediate();
			ActiveSlamChargeComp = nullptr;
		}

		ActiveSlamChargeComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, SlamChargeEffect, SpawnLoc, SpawnRot,
			FVector(1.f), false, true);   // bAutoDestroy = false (직접 제거)
	}
 
	// 부채꼴 데칼 인디케이터 (전체 GroundSlam 범위 표시)
	if (ActivePattern)
	{
		const FVector FootLoc = GetActorLocation()
			- FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		// WorldPosition 기반 재질 → 보스 발 위치 그대로 전달 (오프셋 불필요)
		SpawnSectorDecal(SlamIndicatorDecal,
			FootLoc,
			ActivePattern->ForwardSlamLength,
			ActivePattern->ForwardSlamAngle);
	}

	// 현재 재생 중인 몽타주 일시정지
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		UAnimMontage* CurMontage = AnimInst->GetCurrentActiveMontage();
		if (CurMontage)
		{
			AnimInst->Montage_Pause(CurMontage);
		}
	}

	// 1~2초 랜덤 후 몽타주 재개
	const float ChargeTime = FMath::FRandRange(1.f, 2.f);
	GetWorldTimerManager().ClearTimer(SlamChargeTimer);
	GetWorldTimerManager().SetTimer(
		SlamChargeTimer,
		this,
		&AOHSMBossBase::ResumeSlamMontage,
		ChargeTime,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("[Boss] 차징 시작 — %.1f초 후 재개"), ChargeTime);
}

void AOHSMBossBase::ResumeSlamMontage()
{
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		UAnimMontage* CurMontage = AnimInst->GetCurrentActiveMontage();
		if (CurMontage)
		{
			AnimInst->Montage_Resume(CurMontage);
			UE_LOG(LogTemp, Log, TEXT("[Boss] 차징 완료 — 몽타주 재개"));
		}
	}
}

// ── 실제 파동 발동 (내려찍는 순간 노티파이) ──────────────────────────────
void AOHSMBossBase::TriggerGroundSlam()
{
	if (!ActivePattern || !GetWorld())
	{
		return;
	}

	// 인디케이터 제거
	ClearDecal(SlamIndicatorDecal);

	// 차징 이펙트 즉시 제거
	if (IsValid(ActiveSlamChargeComp))
	{
		ActiveSlamChargeComp->DeactivateImmediate();
		ActiveSlamChargeComp->DestroyComponent();
		ActiveSlamChargeComp = nullptr;
	}

	// 이전 타이머 전부 정리
	for (FTimerHandle& Handle : SlamWaveTimers)
	{
		GetWorldTimerManager().ClearTimer(Handle);
	}
	SlamWaveTimers.Reset();

	const float SlamLength   = ActivePattern->ForwardSlamLength;
	const float HalfAngle    = ActivePattern->ForwardSlamAngle;
	const int32 WaveCount    = FMath::Max(ActivePattern->SlamWaveCount, 1);
	const float WaveInterval = FMath::Max(ActivePattern->SlamWaveInterval, 0.05f);
	const float Damage       = ActivePattern->Damage * GetDamageMultiplier();

	SlamWaveTimers.SetNum(WaveCount);

	for (int32 i = 0; i < WaveCount; ++i)
	{
		const float FireDelay = (i == 0) ? KINDA_SMALL_NUMBER : WaveInterval * i;

		GetWorldTimerManager().SetTimer(
			SlamWaveTimers[i],
			FTimerDelegate::CreateUObject(
				this, &AOHSMBossBase::ExecuteSlamWave,
				i, SlamLength, HalfAngle, Damage, WaveCount),
			FireDelay,
			false
		);
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Boss] 지면 강타 파동 발동 — %d파동, 간격 %.2fs, 최대거리 %.0f"),
		WaveCount, WaveInterval, SlamLength);
}


void AOHSMBossBase::ExecuteSlamWave(
	int32 WaveIndex, float SlamLength, float HalfAngleDeg, float Damage, int32 TotalWaves)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 이번 파동의 외곽·내경 반경
	// 예) WaveCount=3: 파동0→0~200, 파동1→200~400, 파동2→400~600
	const float MaxRadius = SlamLength * (WaveIndex + 1) / TotalWaves;
	const float MinRadius = (WaveIndex > 0) ? SlamLength * WaveIndex / TotalWaves : 0.f;

	const FVector Origin  = GetActorLocation();
	const FVector Fwd     = GetActorForwardVector();
	const FVector Fwd2D   = FVector(Fwd.X, Fwd.Y, 0.f).GetSafeNormal();
	const float   CosHalf = FMath::Cos(FMath::DegreesToRadians(HalfAngleDeg));

	// ── 구 오버랩으로 후보 수집 ─────────────────────────────────────────────
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	World->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(MaxRadius),
		Params
	);

	// ── 링(고리) + 부채꼴 각도 필터 → 데미지 ───────────────────────────────
	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActors.Contains(HitActor)) continue;
		if (!HitActor->ActorHasTag(TEXT("Player")))    continue;

		const FVector ToActor  = HitActor->GetActorLocation() - Origin;
		const float   Dist2D   = FVector(ToActor.X, ToActor.Y, 0.f).Size();

		// 이번 파동 고리 범위 체크
		if (Dist2D < MinRadius || Dist2D > MaxRadius) continue;

		// 부채꼴 각도 체크 (수평 방향만)
		const FVector Dir2D = FVector(ToActor.X, ToActor.Y, 0.f).GetSafeNormal();
		if (FVector::DotProduct(Fwd2D, Dir2D) < CosHalf) continue;

		HitActors.Add(HitActor);
		UGameplayStatics::ApplyDamage(HitActor, Damage, GetController(), this, nullptr);

		UE_LOG(LogTemp, Log, TEXT("[Boss] 지면강타 파동%d 데미지 %.0f → %s"),
			WaveIndex + 1, Damage, *HitActor->GetName());
	}

	// ── 충격파 Niagara 이펙트 — 부채꼴 호 위치에 분산 스폰 ─────────────────
	// 파동마다 MidDist(링 중간 거리) 위치에 스폰 → 펑 펑 펑 앞으로 퍼지는 느낌
	if (SlamWaveEffect)
	{
		const float FootZ   = Origin.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector FootBase = FVector(Origin.X, Origin.Y, FootZ);

		// 이 파동의 링 중간 거리 (ex. 파동0=100, 파동1=300, 파동2=500)
		const float MidDist = (MinRadius + MaxRadius) * 0.5f;

		// 부채꼴을 따라 30°마다 1개씩 분산 스폰
		const int32 SpawnCount = FMath::Max(FMath::RoundToInt(HalfAngleDeg / 30.f) + 1, 3);

		for (int32 s = 0; s < SpawnCount; ++s)
		{
			const float Frac     = (SpawnCount > 1) ? (float)s / (SpawnCount - 1) : 0.5f;
			const float AngleDeg = FMath::Lerp(-HalfAngleDeg, HalfAngleDeg, Frac);
			const FQuat  PointRot = FQuat(FVector::UpVector, FMath::DegreesToRadians(AngleDeg));
			const FVector Dir     = PointRot.RotateVector(Fwd2D);

			// 각 스폰 위치 = 보스 발 + 부채꼴 방향 × 파동 거리
			const FVector SpawnPos = FootBase + Dir * MidDist;

			UNiagaraComponent* WaveComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				SlamWaveEffect,
				SpawnPos,
				Dir.Rotation()   // 이펙트가 바깥 방향을 향하도록
			);
			if (IsValid(WaveComp))
			{
				// 링 너비의 절반 → 이웃 스폰끼리 자연스럽게 이어짐
				WaveComp->SetFloatParameter(TEXT("User.Radius"), (MaxRadius - MinRadius) * 0.4f);
			}
		}
	}

	// ── 디버그 시각화 (에디터 전용) ─────────────────────────────────────────

	UE_LOG(LogTemp, Log, TEXT("[Boss] 파동 %d/%d 완료 — 반경 %.0f~%.0f, 피격 %d명"),
		WaveIndex + 1, TotalWaves, MinRadius, MaxRadius, HitActors.Num());
}

void AOHSMBossBase::ClearDecal(TObjectPtr<UDecalComponent>& Decal)
{
	if (IsValid(Decal))
	{
		Decal->DestroyComponent();
		Decal = nullptr;
	}
}

static FVector GetGroundLocation(UWorld* World, FVector Origin, AActor* Ignore)
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Ignore);
	if (World->LineTraceSingleByChannel(
		Hit,
		Origin + FVector(0,0,200.f),
		Origin - FVector(0,0,800.f),
		ECC_WorldStatic, Params))
	{
		return Hit.Location;
	}
	return Origin;
}

void AOHSMBossBase::SpawnCircleDecal(TObjectPtr<UDecalComponent>& OutDecal, FVector WorldLocation, float Radius) 
{
	ClearDecal(OutDecal);
	
	if (!AttackIndicatorMaterial || !GetWorld())
	{
		return;
	}

	const FVector GroundLoc = GetGroundLocation(GetWorld(), WorldLocation, this);
	
	const FVector DecalSize = FVector(500.f, Radius, Radius);
	
	OutDecal = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		AttackIndicatorMaterial,
		DecalSize,
		GroundLoc,
		FRotator(-90.f, 0.f, 0.f)
	);

	if (IsValid(OutDecal))
	{
		OutDecal->SetFadeOut(0.f, 0.f, false);
	}
}

void AOHSMBossBase::SpawnSectorDecal(TObjectPtr<UDecalComponent>& OutDecal, FVector FootLocation, float Length, float HalfAngleDeg) 
{
	ClearDecal(OutDecal);
	
	if (!SectorIndicatorMaterial || !GetWorld())
	{
		return;
	}
	UMaterialInstanceDynamic* DMI = UMaterialInstanceDynamic::Create(SectorIndicatorMaterial, this);
	if (!DMI)
	{
		return;
	}
	const FVector Forward = GetActorForwardVector();
	const float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(HalfAngleDeg));
	
	DMI->SetVectorParameterValue(TEXT("BossWorldPos"),
		FLinearColor(FootLocation.X, FootLocation.Y, FootLocation.Z, 0.f));
	DMI->SetVectorParameterValue(TEXT("SectorForward"),
		FLinearColor(Forward.X, Forward.Y, 0.f, 0.f));
	DMI->SetScalarParameterValue(TEXT("HalfAngleCos"), HalfAngleCos);
	DMI->SetScalarParameterValue(TEXT("MaxRadius"), Length);
	
	const FVector GroundLoc  = GetGroundLocation(GetWorld(), FootLocation, this);
	const FVector DecalSize  = FVector(500.f, Length, Length);
	
	OutDecal = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(), DMI, DecalSize, GroundLoc, FRotator(-90.f, 0.f, 0.f));

	if (IsValid(OutDecal))
	{
		OutDecal->SetFadeOut(0.f, 0.f, false);
	}
}

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

}

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

		// 원거리 상한 거리
		if (bIsRangedType && Pattern->MaxAttackRange > 0.f &&
			DistToTarget > Pattern->MaxAttackRange)
		{
			continue;
		}

		return true; // 사용 가능한 패턴 하나 이상 존재
	}

	return false;
}
