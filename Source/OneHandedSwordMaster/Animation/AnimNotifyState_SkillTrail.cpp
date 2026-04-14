// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_SkillTrail.h"

#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

void UAnimNotifyState_SkillTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->ActivateSkillTrail(TrailSystem);
	}
}

void UAnimNotifyState_SkillTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->DeactivateSkillTrail();
	}
}
