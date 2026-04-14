// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMBuffSkill.h"

#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"

void UOHSMBuffSkill::ActivateSkill_Implementation(ACharacter* Instigator)
{
	if (!IsValid(Instigator) || !SkillData)
	{
		return;
	}

	UOHSMPlayerStatComponent* StatComp =
		Instigator->FindComponentByClass<UOHSMPlayerStatComponent>();
	if (!IsValid(StatComp))
	{
		return;
	}

	// 이전 버프가 남아 있으면 먼저 제거
	if (BuffTimerHandle.IsValid())
	{
		if (UWorld* W = GetWorld())
		{
			W->GetTimerManager().ClearTimer(BuffTimerHandle);
		}
		if (ACharacter* Prev = BuffedCharacter.Get())
		{
			if (UOHSMPlayerStatComponent* PrevStat =
					Prev->FindComponentByClass<UOHSMPlayerStatComponent>())
			{
				PrevStat->RemoveTemporaryBonus(CachedStatType, CachedEffectValue);
			}
		}
	}

	CachedEffectValue = SkillData->EffectValue;
	CachedStatType    = SkillData->BuffStatType;
	BuffedCharacter   = Instigator;

	// 버프 스탯 즉시 적용
	StatComp->AddTemporaryBonus(CachedStatType, CachedEffectValue);

	// 발동 이펙트 (1회 재생)
	SpawnActivateEffect(Instigator);

	// 몽타주 재생 + 몽타주 종료까지 이동 잠금
	if (IsValid(BuffMontage))
	{
		PlayMontageAndBlockMovement(Instigator, BuffMontage, PlayRate);
	}

	// 루프 이펙트 — 버프 지속 중 캐릭터에 붙어 재생
	if (IsValid(BuffLoopEffect))
	{
		// 이전 루프 이펙트가 있으면 먼저 제거
		if (IsValid(ActiveLoopComponent))
		{
			ActiveLoopComponent->DestroyComponent();
			ActiveLoopComponent = nullptr;
		}

		USkeletalMeshComponent* Mesh = Instigator->GetMesh();
		if (IsValid(Mesh))
		{
			const FName Socket = (BuffLoopSocketName != NAME_None && Mesh->DoesSocketExist(BuffLoopSocketName))
				? BuffLoopSocketName
				: NAME_None;

			ActiveLoopComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				BuffLoopEffect,
				Mesh,
				Socket,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false   // bAutoDestroy = false (만료 시 수동 제거)
			);
		}
	}

	// 버프 지속 시간 타이머 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BuffTimerHandle,
			this, &UOHSMBuffSkill::OnBuffExpired,
			SkillData->Duration,
			false
		);
	}
}

void UOHSMBuffSkill::OnBuffExpired()
{
	if (ACharacter* Char = BuffedCharacter.Get())
	{
		if (UOHSMPlayerStatComponent* StatComp =
				Char->FindComponentByClass<UOHSMPlayerStatComponent>())
		{
			StatComp->RemoveTemporaryBonus(CachedStatType, CachedEffectValue);
		}
	}

	// 루프 이펙트 제거
	if (IsValid(ActiveLoopComponent))
	{
		ActiveLoopComponent->DestroyComponent();
		ActiveLoopComponent = nullptr;
	}

	// 버프 만료 — 스킬 컴포넌트에 알려서 UI 제거
	if (UOHSMSkillComponent* SkillComp = Cast<UOHSMSkillComponent>(GetOuter()))
	{
		SkillComp->BroadcastBuffExpired(SkillID);
	}

	BuffedCharacter.Reset();
	CachedEffectValue = 0.f;
}
