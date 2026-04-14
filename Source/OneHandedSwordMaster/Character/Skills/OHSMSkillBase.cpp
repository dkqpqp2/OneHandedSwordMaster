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

	// 소켓이 지정돼 있으면 소켓에 Attach, 없으면 캐릭터 위치에 월드 스폰
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

	CachedCharacter = Char;
	CachedMontage   = Montage;

	// 이동 비활성화 (MOVE_None)
	Char->GetCharacterMovement()->DisableMovement();

	// 중복 바인딩 방지 — 이전 바인딩 먼저 제거 후 재등록
	AnimInst->OnMontageEnded.RemoveDynamic(this, &UOHSMSkillBase::OnMontageEnded);
	AnimInst->OnMontageEnded.AddDynamic(this, &UOHSMSkillBase::OnMontageEnded);

	// 몽타주 재생
	AnimInst->Montage_Play(Montage, InPlayRate);
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
		// 이동 복구 (Walking 모드)
		CachedCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

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
