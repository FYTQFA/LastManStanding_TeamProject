#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LMSPlayerController.generated.h"

class UUIManagerComponent;
class ULMSCombatHUDWidget;
class ULMSAttributeSet;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;

UCLASS()
class LMS_TEAMPROJECT_API ALMSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALMSPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

private:
	// 전투 HUD 위젯을 찾고, PlayerState의 Attribute 값을 HUD에 전달할 준비를 합니다.
	void InitializeCombatHUD();

	// UIManagerComponent가 생성한 전투 HUD를 C++ 부모 클래스 타입으로 캐싱합니다.
	void CacheCombatHUDWidget();

	// PlayerState가 가진 AbilitySystemComponent와 AttributeSet을 캐싱합니다.
	void CacheAttributeSource();

	// GAS Attribute 값이 바뀔 때 HUD를 갱신하도록 델리게이트를 연결합니다.
	void BindAttributeDelegates();

	// 현재 Attribute 값을 기준으로 HUD 전체를 한 번 갱신합니다.
	void UpdateAllCombatHUD();

	// 체력/쉴드/스테미나 각각의 HUD 표시를 갱신합니다.
	void UpdateHealthUI();
	void UpdateShieldUI();
	void UpdateStaminaUI();

	// GAS Attribute 변경 알림을 받으면 해당 HUD 표시를 다시 갱신합니다.
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleShieldChanged(const FOnAttributeChangeData& Data);
	void HandleMaxShieldChanged(const FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const FOnAttributeChangeData& Data);
	void HandleMaxStaminaChanged(const FOnAttributeChangeData& Data);

	// 전투 UI, 로비 UI, 리스폰 UI, 일시정지 UI 등을 생성/표시/숨김 처리하는 UI 관리자 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LMS|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUIManagerComponent> UIManagerComponent;

	// UIManagerComponent가 생성한 실제 전투 HUD 위젯입니다.
	UPROPERTY()
	TObjectPtr<ULMSCombatHUDWidget> CombatHUDWidget;

	// PlayerState에 있는 GAS 컴포넌트와 AttributeSet입니다.
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<ULMSAttributeSet> CachedAttributeSet;

	// 델리게이트가 중복으로 연결되는 것을 막기 위한 플래그입니다.
	bool bAttributeDelegatesBound = false;
};
