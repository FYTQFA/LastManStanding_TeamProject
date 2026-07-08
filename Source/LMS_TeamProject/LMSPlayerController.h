#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LMSPlayerController.generated.h"

class UUIManagerComponent;
class ULMSCombatHUDPresenterComponent;

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
	// 전투 UI, 로비 UI, 리스폰 UI, 일시정지 UI 등을 생성/표시/숨김 처리하는 UI 관리자 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LMS|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUIManagerComponent> UIManagerComponent;

	// 전투 HUD에 필요한 게임 데이터를 실제 위젯으로 전달하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LMS|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULMSCombatHUDPresenterComponent> CombatHUDPresenterComponent;
};
