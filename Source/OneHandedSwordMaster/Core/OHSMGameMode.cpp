// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMGameMode.h"

#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

AOHSMGameMode::AOHSMGameMode()
{
	static ConstructorHelpers::FClassFinder<AOHSMPlayerCharacter> PlayerCharacterRef(TEXT("'/Game/OneHandedSwordMaster/Character/BP_OHSMPlayerCharacter.BP_OHSMPlayerCharacter_C'"));
	if (PlayerCharacterRef.Class)
	{
		DefaultPawnClass = PlayerCharacterRef.Class;
	}
	
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/OneHandedSwordMaster.OHSMPlayerController"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}
}
