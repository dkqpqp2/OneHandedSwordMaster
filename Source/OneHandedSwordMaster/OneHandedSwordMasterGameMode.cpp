// Copyright Epic Games, Inc. All Rights Reserved.

#include "OneHandedSwordMasterGameMode.h"
#include "OneHandedSwordMasterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AOneHandedSwordMasterGameMode::AOneHandedSwordMasterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
