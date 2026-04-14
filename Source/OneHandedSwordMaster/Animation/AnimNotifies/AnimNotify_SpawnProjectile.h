// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SpawnProjectile.generated.h"

/**
 * 몽타주의 원하는 타이밍에 배치하면 AOHSMPlayerCharacter::SpawnPreparedProjectile()을 호출해
 * OHSMSlashSkill이 미리 저장해둔 투사체를 실제로 스폰한다.
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UAnimNotify_SpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
