#include "LMSCombatHUDPresenterComponent.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "../LMSAttributeSet.h"
#include "../LMS_TeamProjectPlayerState.h"
#include "LMSCombatHUDWidget.h"
#include "UIManagerComponent.h"

ULMSCombatHUDPresenterComponent::ULMSCombatHUDPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULMSCombatHUDPresenterComponent::InitializeCombatHUD()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	CacheUIManagerComponent();
	CacheCombatHUDWidget();
	CacheAttributeSource();
	BindAttributeDelegates();
	UpdateAllCombatHUD();
}

void ULMSCombatHUDPresenterComponent::CacheUIManagerComponent()
{
	if (UIManagerComponent)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController)
	{
		return;
	}

	UIManagerComponent = PlayerController->FindComponentByClass<UUIManagerComponent>();
}

void ULMSCombatHUDPresenterComponent::CacheCombatHUDWidget()
{
	if (CombatHUDWidget || !UIManagerComponent)
	{
		return;
	}

	// UIManagerComponent가 Combat 타입으로 생성해 둔 위젯을 가져옵니다.
	// 이 위젯은 WBP_CombatHUD이고, 부모 클래스가 ULMSCombatHUDWidget이어야 합니다.
	CombatHUDWidget = Cast<ULMSCombatHUDWidget>(UIManagerComponent->GetUI(ELMSUIType::Combat));
}

void ULMSCombatHUDPresenterComponent::CacheAttributeSource()
{
	if (CachedAbilitySystemComponent && CachedAttributeSet)
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController)
	{
		return;
	}

	ALMS_TeamProjectPlayerState* LMSPlayerState = PlayerController->GetPlayerState<ALMS_TeamProjectPlayerState>();
	if (!LMSPlayerState)
	{
		return;
	}

	CachedAbilitySystemComponent = LMSPlayerState->GetAbilitySystemComponent();
	CachedAttributeSet = LMSPlayerState->GetAttributeSet();
}

void ULMSCombatHUDPresenterComponent::BindAttributeDelegates()
{
	if (bAttributeDelegatesBound || !CachedAbilitySystemComponent || !CachedAttributeSet)
	{
		return;
	}

	// GAS가 제공하는 Attribute 변경 델리게이트에 UI 갱신 함수를 연결합니다.
	// 값이 바뀔 때만 HUD를 갱신하므로 Tick으로 매 프레임 읽는 방식보다 부담이 적습니다.
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetHealthAttribute())
		.AddUObject(this, &ULMSCombatHUDPresenterComponent::HandleHealthChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetMaxHealthAttribute())
		.AddUObject(this, &ULMSCombatHUDPresenterComponent::HandleMaxHealthChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetShieldAttribute())
		.AddUObject(this, &ULMSCombatHUDPresenterComponent::HandleShieldChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetMaxShieldAttribute())
		.AddUObject(this, &ULMSCombatHUDPresenterComponent::HandleMaxShieldChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetStaminaAttribute())
		.AddUObject(this, &ULMSCombatHUDPresenterComponent::HandleStaminaChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetMaxStaminaAttribute())
		.AddUObject(this, &ULMSCombatHUDPresenterComponent::HandleMaxStaminaChanged);

	bAttributeDelegatesBound = true;
}

void ULMSCombatHUDPresenterComponent::UpdateAllCombatHUD()
{
	UpdateHealthUI();
	UpdateShieldUI();
	UpdateStaminaUI();
}

void ULMSCombatHUDPresenterComponent::UpdateHealthUI()
{
	if (!CombatHUDWidget || !CachedAttributeSet)
	{
		return;
	}

	CombatHUDWidget->SetHealth(CachedAttributeSet->GetHealth(), CachedAttributeSet->GetMaxHealth());
}

void ULMSCombatHUDPresenterComponent::UpdateShieldUI()
{
	if (!CombatHUDWidget || !CachedAttributeSet)
	{
		return;
	}

	CombatHUDWidget->SetShield(CachedAttributeSet->GetShield(), CachedAttributeSet->GetMaxShield());
}

void ULMSCombatHUDPresenterComponent::UpdateStaminaUI()
{
	if (!CombatHUDWidget || !CachedAttributeSet)
	{
		return;
	}

	CombatHUDWidget->SetStamina(CachedAttributeSet->GetStamina(), CachedAttributeSet->GetMaxStamina());
}

void ULMSCombatHUDPresenterComponent::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthUI();
}

void ULMSCombatHUDPresenterComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthUI();
}

void ULMSCombatHUDPresenterComponent::HandleShieldChanged(const FOnAttributeChangeData& Data)
{
	UpdateShieldUI();
}

void ULMSCombatHUDPresenterComponent::HandleMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	UpdateShieldUI();
}

void ULMSCombatHUDPresenterComponent::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaUI();
}

void ULMSCombatHUDPresenterComponent::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaUI();
}

void ULMSCombatHUDPresenterComponent::UpdateSkillCooldownUI(float CurrentCooldown, float MaxCooldown) const
{
	if (!CombatHUDWidget)
	{
		return;
	}

	CombatHUDWidget->SetSkillCooldown(CurrentCooldown, MaxCooldown);
}
