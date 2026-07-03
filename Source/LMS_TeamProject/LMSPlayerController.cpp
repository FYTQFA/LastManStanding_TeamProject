#include "LMSPlayerController.h"
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
}