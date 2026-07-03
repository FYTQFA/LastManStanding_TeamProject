// Copyright Epic Games, Inc. All Rights Reserved.

#include "LMS_TeamProjectGameMode.h"
#include "LMS_TeamProjectCharacter.h"
#include "LMS_TeamProjectPlayerState.h"
#include "UObject/ConstructorHelpers.h"

ALMS_TeamProjectGameMode::ALMS_TeamProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}


	PlayerStateClass = ALMS_TeamProjectPlayerState::StaticClass();

}
