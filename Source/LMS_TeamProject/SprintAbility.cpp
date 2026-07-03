#include "SprintAbility.h"
#include "AbilitySystemComponent.h"

USprintAbility::USprintAbility()
{
	// 부모(LMSGameplayAbility)에서 이미 InstancedPerActor + LocalPredicted 세팅됨
}

void USprintAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 코스트/쿨다운 통과 못 하면 여기서 자동 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC || !SprintEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// GE 적용 (서버에서만 실제 적용됨 — Speed는 복제로 클라 반영)
	if (HasAuthority(&ActivationInfo))
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(SprintEffectClass, 1.f, Context);

		if (SpecHandle.IsValid())
		{
			ActiveSprintHandle =
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void USprintAbility::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	// 키 떼면 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void USprintAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// GE 제거 (서버에서 적용했으니 서버에서 제거)
	if (HasAuthority(&ActivationInfo) && ActiveSprintHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(ActiveSprintHandle);
		}
		ActiveSprintHandle = FActiveGameplayEffectHandle();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}