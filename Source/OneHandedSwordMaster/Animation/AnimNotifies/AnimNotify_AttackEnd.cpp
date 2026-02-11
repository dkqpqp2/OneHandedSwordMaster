// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttackEnd.h"

#include "OneHandedSwordMaster/Character/Components/OHSMCombatComponent.h"

void UAnimNotify_AttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	
}
