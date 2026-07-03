// Copyright Epic Games, Inc. All Rights Reserved.

#include "LMSAttributeSet.h"
#include "Net/UnrealNetwork.h"

ULMSAttributeSet::ULMSAttributeSet()
	: Health(100.f)
	, MaxHealth(100.f)
	, Shield(100.f)
	, MaxShield(100.f)
	, Stamina(100.f)
	, MaxStamina(100.f)
	, Speed(500.f)
	, MaxSpeed(1000.f)
{
}

void ULMSAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, Speed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULMSAttributeSet, MaxSpeed, COND_None, REPNOTIFY_Always);

}

void ULMSAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
	}
	else if (Attribute == GetMaxShieldAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSpeed());
	}
	else if (Attribute == GetMaxSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}


} //GetMax...에서 Max의 최솟값을 1.f로 설정 -> 0 나누기 방지
//ApplyGameplayEffects -> PreAttributeChange(엔진이 자동으로 값 검사) -> 검사 완료된 값 GE로 반영

void ULMSAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, Health, OldHealth);
	// GAS한테 "이 값 복제로 바뀜, 옛값은 이거" 통지
}

void ULMSAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, MaxHealth, OldMaxHealth);
}

void ULMSAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, Shield, OldShield);
}

void ULMSAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, MaxShield, OldMaxShield);
}

void ULMSAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, Stamina, OldStamina);
}

void ULMSAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, MaxStamina, OldMaxStamina);
}

void ULMSAttributeSet::OnRep_Speed(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, Speed, OldStamina);
}

void ULMSAttributeSet::OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULMSAttributeSet, MaxSpeed, OldMaxStamina);
}