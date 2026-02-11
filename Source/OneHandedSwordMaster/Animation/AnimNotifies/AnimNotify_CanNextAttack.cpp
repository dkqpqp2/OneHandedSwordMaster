// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_CanNextAttack.h"

#include "OneHandedSwordMaster/Character/Components/OHSMCombatComponent.h"

void UAnimNotify_CanNextAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, TEXT("여기는 나오나?"));
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UOHSMCombatComponent* CombatComp = Owner->FindComponentByClass<UOHSMCombatComponent>())
		{
			CombatComp->EnableNextAttackInput();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[애님노티파이] CombatComponent를 찾을 수 없습니다!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[애님노티파이] Owner가 null입니다!"));
	}
}
