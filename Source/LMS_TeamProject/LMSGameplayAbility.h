// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LMSGameplayAbility.generated.h"

/**
 * Base gameplay ability class for LMS_TeamProject. Project-specific abilities should derive from this.
 */
UCLASS()
class LMS_TEAMPROJECT_API ULMSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	ULMSGameplayAbility();
};
