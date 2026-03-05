// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMGameMode.h"

#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

AOHSMGameMode::AOHSMGameMode()
{
	DefaultPawnClass = AOHSMPlayerCharacter::StaticClass();
}
