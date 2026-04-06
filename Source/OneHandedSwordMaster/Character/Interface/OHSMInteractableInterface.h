#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OHSMInteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UOHSMInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class ONEHANDEDSWORDMASTER_API IOHSMInteractableInterface
{
	GENERATED_BODY()

public:
	/** 플레이어가 상호작용 키를 눌렀을 때 호출 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(APlayerController* InstigatorController);
};
