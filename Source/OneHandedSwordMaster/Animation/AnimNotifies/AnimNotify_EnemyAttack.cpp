// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_EnemyAttack.h"

#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"

void UAnimNotify_EnemyAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(MeshComp->GetOwner());
	if (!IsValid(Enemy))
	{
		return;
	}

	Enemy->PerformAttack(bIsAreaAttack, AttackRadius);
}
