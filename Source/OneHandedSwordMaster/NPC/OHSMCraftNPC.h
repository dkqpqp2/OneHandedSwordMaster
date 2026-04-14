#pragma once

#include "CoreMinimal.h"
#include "OHSMNPCBase.h"
#include "OHSMCraftNPC.generated.h"

UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMCraftNPC : public AOHSMNPCBase
{
	GENERATED_BODY()

public:
	AOHSMCraftNPC();

protected:
	/** 확인 버튼 → 제작창 오픈 */
	virtual void OnDialogueConfirmed(APlayerController* InstigatorController) override;
};
