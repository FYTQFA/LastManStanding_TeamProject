#include "LMSPlayerController.h"

#include "AbilitySystemComponent.h"
#include "LMSAttributeSet.h"
#include "LMS_TeamProjectPlayerState.h"
#include "UI/LMSCombatHUDWidget.h"
#include "UI/UIManagerComponent.h"

ALMSPlayerController::ALMSPlayerController()
{
	// PlayerController가 생성될 때 UIManagerComponent도 함께 생성합니다.
	UIManagerComponent = CreateDefaultSubobject<UUIManagerComponent>(TEXT("UIManagerComponent"));
}

void ALMSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 멀티플레이에서 로컬 플레이어 Controller만 UI를 띄워야 합니다.
	if (!IsLocalController())
	{
		return;
	}

	if (UIManagerComponent)
	{
		UIManagerComponent->ShowUI(ELMSUIType::Combat);
	}

	InitializeCombatHUD();
}

void ALMSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서는 PlayerState가 BeginPlay보다 늦게 복제될 수 있습니다.
	// 그 경우 PlayerState가 준비된 뒤 다시 HUD 연결을 시도합니다.
	InitializeCombatHUD();
}

void ALMSPlayerController::InitializeCombatHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	CacheCombatHUDWidget();
	CacheAttributeSource();
	BindAttributeDelegates();
	UpdateAllCombatHUD();
}

void ALMSPlayerController::CacheCombatHUDWidget()
{
	if (CombatHUDWidget || !UIManagerComponent)
	{
		return;
	}

	// UIManagerComponent가 Combat 타입으로 생성해 둔 위젯을 가져옵니다.
	// 이 위젯은 WBP_CombatHUD이고, 부모 클래스가 ULMSCombatHUDWidget이어야 합니다.
	CombatHUDWidget = Cast<ULMSCombatHUDWidget>(UIManagerComponent->GetUI(ELMSUIType::Combat));
}

void ALMSPlayerController::CacheAttributeSource()
{
	if (CachedAbilitySystemComponent && CachedAttributeSet)
	{
		return;
	}

	ALMS_TeamProjectPlayerState* LMSPlayerState = GetPlayerState<ALMS_TeamProjectPlayerState>();
	if (!LMSPlayerState)
	{
		return;
	}

	CachedAbilitySystemComponent = LMSPlayerState->GetAbilitySystemComponent();
	CachedAttributeSet = LMSPlayerState->GetAttributeSet();
}

void ALMSPlayerController::BindAttributeDelegates()
{
	if (bAttributeDelegatesBound || !CachedAbilitySystemComponent || !CachedAttributeSet)
	{
		return;
	}

	// GAS가 제공하는 Attribute 변경 델리게이트에 UI 갱신 함수를 연결합니다.
	// 값이 바뀔 때만 HUD를 갱신하므로 Tick으로 매 프레임 읽는 방식보다 부담이 적습니다.
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetHealthAttribute())
		.AddUObject(this, &ALMSPlayerController::HandleHealthChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetMaxHealthAttribute())
		.AddUObject(this, &ALMSPlayerController::HandleMaxHealthChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetShieldAttribute())
		.AddUObject(this, &ALMSPlayerController::HandleShieldChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetMaxShieldAttribute())
		.AddUObject(this, &ALMSPlayerController::HandleMaxShieldChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetStaminaAttribute())
		.AddUObject(this, &ALMSPlayerController::HandleStaminaChanged);
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedAttributeSet->GetMaxStaminaAttribute())
		.AddUObject(this, &ALMSPlayerController::HandleMaxStaminaChanged);

	bAttributeDelegatesBound = true;
}

void ALMSPlayerController::UpdateAllCombatHUD()
{
	UpdateHealthUI();
	UpdateShieldUI();
	UpdateStaminaUI();
}

void ALMSPlayerController::UpdateHealthUI()
{
	if (!CombatHUDWidget || !CachedAttributeSet)
	{
		return;
	}

	CombatHUDWidget->SetHealth(CachedAttributeSet->GetHealth(), CachedAttributeSet->GetMaxHealth());
}

void ALMSPlayerController::UpdateShieldUI()
{
	if (!CombatHUDWidget || !CachedAttributeSet)
	{
		return;
	}

	CombatHUDWidget->SetShield(CachedAttributeSet->GetShield(), CachedAttributeSet->GetMaxShield());
}

void ALMSPlayerController::UpdateStaminaUI()
{
	if (!CombatHUDWidget || !CachedAttributeSet)
	{
		return;
	}

	CombatHUDWidget->SetStamina(CachedAttributeSet->GetStamina(), CachedAttributeSet->GetMaxStamina());
}

void ALMSPlayerController::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthUI();
}

void ALMSPlayerController::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthUI();
}

void ALMSPlayerController::HandleShieldChanged(const FOnAttributeChangeData& Data)
{
	UpdateShieldUI();
}

void ALMSPlayerController::HandleMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	UpdateShieldUI();
}

void ALMSPlayerController::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaUI();
}

void ALMSPlayerController::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaUI();
}
