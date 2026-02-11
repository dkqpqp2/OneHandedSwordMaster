// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_WeaponTrace.h"

#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Weapon/OHSMWeaponBase.h"

void UAnimNotifyState_WeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                               float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (AOHSMWeaponBase* Weapon = GetWeaponFromCharacter(MeshComp->GetOwner()))
	{
		Weapon->StartTracing();
	}
}

void UAnimNotifyState_WeaponTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (AOHSMWeaponBase* Weapon = GetWeaponFromCharacter(MeshComp->GetOwner()))
	{
		Weapon->PerformTrace();
	}
}

void UAnimNotifyState_WeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AOHSMWeaponBase* Weapon = GetWeaponFromCharacter(MeshComp->GetOwner()))
	{
		Weapon->StopTracing();
	}
}

TObjectPtr<class AOHSMWeaponBase> UAnimNotifyState_WeaponTrace::GetWeaponFromCharacter(AActor* Character)
{
	if (!Character)
	{
		return nullptr;
	}
	
	if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(Character))
	{
		return Player->GetCurrentWeapon();
	}
	
	return nullptr;
}
