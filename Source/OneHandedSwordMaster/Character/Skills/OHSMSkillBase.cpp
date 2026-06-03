// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMSkillBase.h"

#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UWorld* UOHSMSkillBase::GetWorld() const
{
	// Outer가 UActorComponent이면 그 World를 반환
	if (UActorComponent* Outer = Cast<UActorComponent>(GetOuter()))
	{
		return Outer->GetWorld();
	}
	return nullptr;
}

// ─── 나이아가라 이펙트 ──────────────────────────────────────────────────────

// 발동 이펙트 스폰
void UOHSMSkillBase::SpawnActivateEffect(ACharacter* Char)
{
	if (!IsValid(Char) || !IsValid(ActivateEffect))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Char->GetMesh();
	if (!IsValid(Mesh))
	{
		return;
	}

	// 소켓이 있으면 소켓 위치, 없으면 캐릭터 위치에 스폰
	if (EffectSocketName != NAME_None && Mesh->DoesSocketExist(EffectSocketName))
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			ActivateEffect,
			Mesh,
			EffectSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true   // bAutoDestroy
		);
	}
	else
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			Char,
			ActivateEffect,
			Char->GetActorLocation()
		);
	}
}

// ─── 몽타주 재생 + 이동 잠금 ────────────────────────────────────────────────

// 몽타주 재생 + 이동 잠금
void UOHSMSkillBase::PlayMontageAndBlockMovement(ACharacter* Char, UAnimMontage* Montage, float InPlayRate)
{
	if (!IsValid(Char) || !IsValid(Montage))
	{
		return;
	}

	UAnimInstance* AnimInst = Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr;
	if (!IsValid(AnimInst))
	{
		return;
	}

	CachedCharacter = Char; // 캐릭터 캐싱
	CachedMontage   = Montage; // 몽타주 캐싱

	Char->GetCharacterMovement()->DisableMovement(); // 이동 비활성화

	// 중복 바인딩 방지
	AnimInst->OnMontageEnded.RemoveDynamic(this, &UOHSMSkillBase::OnMontageEnded);
	AnimInst->OnMontageEnded.AddDynamic(this, &UOHSMSkillBase::OnMontageEnded);

	AnimInst->Montage_Play(Montage, InPlayRate); // 몽타주 재생
}

void UOHSMSkillBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 이 스킬이 재생한 몽타주가 아니면 무시
	if (Montage != CachedMontage)
	{
		return;
	}

	if (IsValid(CachedCharacter))
	{
		// 이동 복구 — 스킬이 잠근 MOVE_None 상태일 때만 Walking으로 되돌린다.
		// 넉다운(LaunchCharacter)이 이미 MOVE_Falling으로 전환한 경우엔
		// 간섭하지 않아야 Landed() 이벤트가 정상 발동된다.
		UCharacterMovementComponent* MoveComp = CachedCharacter->GetCharacterMovement();
		if (IsValid(MoveComp) && MoveComp->MovementMode == MOVE_None)
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}

		// 델리게이트 해제
		UAnimInstance* AnimInst = CachedCharacter->GetMesh()
			? CachedCharacter->GetMesh()->GetAnimInstance()
			: nullptr;

		if (IsValid(AnimInst))
		{
			AnimInst->OnMontageEnded.RemoveDynamic(this, &UOHSMSkillBase::OnMontageEnded);
		}
	}

	CachedCharacter = nullptr;
	CachedMontage   = nullptr;
}
