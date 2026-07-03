#pragma once

#include "CoreMinimal.h"
#include "LMSGameplayAbility.h"
#include "SprintAbility.generated.h"

UCLASS()
class LMS_TEAMPROJECT_API USprintAbility : public ULMSGameplayAbility
{
	GENERATED_BODY()

public:
	USprintAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	/** 적용할 Sprint GE (BP에서 GE_Sprint 꽂기) */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	TSubclassOf<UGameplayEffect> SprintEffectClass;

	/** 적용된 GE 핸들 (제거용 저장) */
	FActiveGameplayEffectHandle ActiveSprintHandle;
};