// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_WeaponTrail.h"

#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

void UAnimNotifyState_WeaponTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->ActivateWeaponTrail();
	}
}

void UAnimNotifyState_WeaponTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->DeactivateWeaponTrail();
	}
}
