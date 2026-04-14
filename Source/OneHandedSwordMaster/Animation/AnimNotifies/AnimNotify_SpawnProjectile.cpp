// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_SpawnProjectile.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

void UAnimNotify_SpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->SpawnPreparedProjectile();
	}
}
