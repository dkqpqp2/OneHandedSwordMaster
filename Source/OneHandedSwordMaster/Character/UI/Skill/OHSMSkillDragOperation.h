// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "OHSMSkillDragOperation.generated.h"

/**
 * 스킬 노드에서 스킬 퀵슬롯으로 드래그할 때 사용하는 Drag Operation.
 * Payload = 드래그를 시작한 UOHSMSkillNodeWidget 포인터
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMSkillDragOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 드래그 중인 스킬 Row ID */
	UPROPERTY()
	FName SkillID;

	/** 드래그 비주얼에 표시할 아이콘 */
	UPROPERTY()
	TObjectPtr<UTexture2D> SkillIcon;
};
