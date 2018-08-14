// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "GrapplingHookTestGameMode.h"
#include "GrapplingHookTestHUD.h"
#include "GrapplingHookTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGrapplingHookTestGameMode::AGrapplingHookTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AGrapplingHookTestHUD::StaticClass();
}
