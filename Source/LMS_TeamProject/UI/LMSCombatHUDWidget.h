// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LMSCombatHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class LMS_TEAMPROJECT_API ULMSCombatHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 현재 체력과 최대 체력을 HUD에 전달합니다.
	// 실제 ProgressBar/Text 갱신 로직은 WBP_LJH_CombatHUD 블루프린트에서 구현합니다.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "LMS|Combat HUD")
	void SetHealth(float CurrentHealth, float MaxHealth);

	// 현재 쉴드와 최대 쉴드를 HUD에 전달합니다.
	// 실제 ProgressBar/Text 갱신 로직은 WBP_LJH_CombatHUD 블루프린트에서 구현합니다.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "LMS|Combat HUD")
	void SetShield(float CurrentShield, float MaxShield);

	// 현재 스테미나와 최대 스테미나를 HUD에 전달합니다.
	// 실제 ProgressBar/Text 갱신 로직은 WBP_LJH_CombatHUD 블루프린트에서 구현합니다.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "LMS|Combat HUD")
	void SetStamina(float CurrentStamina, float MaxStamina);
	
};
