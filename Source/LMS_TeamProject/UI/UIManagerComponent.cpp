#include "UIManagerComponent.h"
#include "GameFramework/PlayerController.h"

UUIManagerComponent::UUIManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	for (const FLMSUIData& UIData : UIDataList)
	{
		if (UIData.bCreateOnBeginPlay)
		{
			CreateUI(UIData.UIType);
		}

		if (UIData.bShowOnBeginPlay)
		{
			ShowUI(UIData.UIType);
		}
	}
}

UUserWidget* UUIManagerComponent::CreateUI(ELMSUIType UIType)
{
	// 이미 생성된 UI가 있으면 새로 만들지 않고 기존 UI를 반환합니다.
	if (TObjectPtr<UUserWidget>* FoundWidget = CreatedUIMap.Find(UIType))
	{
		return FoundWidget->Get();
	}

	const FLMSUIData* FoundUIData = FindUIData(UIType);

	if (!FoundUIData)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateUI Failed: UIData not found."));
		return nullptr;
	}

	if (!FoundUIData->WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateUI Failed: WidgetClass is null."));
		return nullptr;
	}

	// UIManagerComponent는 PlayerController에 붙이는 구조를 기준으로 합니다.
	// 따라서 Owner가 PlayerController인지 확인합니다.
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateUI Failed: Owner is not PlayerController."));
		return nullptr;
	}

	// 멀티플레이 환경에서 로컬 플레이어의 Controller만 UI를 생성해야 합니다.
	// 서버나 다른 플레이어의 Controller에서 UI가 생성되는 것을 방지합니다.
	if (!PlayerController->IsLocalController())
	{
		return nullptr;
	}

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(PlayerController, FoundUIData->WidgetClass);

	if (!NewWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateUI Failed: CreateWidget returned null."));
		return nullptr;
	}

	NewWidget->AddToPlayerScreen(FoundUIData->ZOrder);
	NewWidget->SetVisibility(ESlateVisibility::Collapsed);

	CreatedUIMap.Add(UIType, NewWidget);

	UE_LOG(LogTemp, Log, TEXT("CreateUI Success."));

	return NewWidget;
}

void UUIManagerComponent::ShowUI(ELMSUIType UIType)
{
	UUserWidget* TargetWidget = GetUI(UIType);

	if (!TargetWidget)
	{
		TargetWidget = CreateUI(UIType);
	}

	if (!TargetWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowUI Failed: TargetWidget is null."));
		return;
	}

	const FLMSUIData* FoundUIData = FindUIData(UIType);

	if (!FoundUIData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowUI Failed: UIData not found."));
		return;
	}

	TargetWidget->SetVisibility(FoundUIData->VisibleState);

	UE_LOG(LogTemp, Log, TEXT("ShowUI Success."));
}

void UUIManagerComponent::HideUI(ELMSUIType UIType)
{
	UUserWidget* TargetWidget = GetUI(UIType);

	if (!TargetWidget)
	{
		return;
	}

	TargetWidget->SetVisibility(ESlateVisibility::Collapsed);

	UE_LOG(LogTemp, Log, TEXT("HideUI Success."));
}

void UUIManagerComponent::ToggleUI(ELMSUIType UIType)
{
	UUserWidget* TargetWidget = GetUI(UIType);

	if (!TargetWidget)
	{
		ShowUI(UIType);
		return;
	}

	if (TargetWidget->GetVisibility() == ESlateVisibility::Collapsed ||
		TargetWidget->GetVisibility() == ESlateVisibility::Hidden)
	{
		ShowUI(UIType);
	}
	else
	{
		HideUI(UIType);
	}
}

void UUIManagerComponent::HideAllUI()
{
	for (const TPair<ELMSUIType, TObjectPtr<UUserWidget>>& UIWidgetPair : CreatedUIMap)
	{
		if (UIWidgetPair.Value)
		{
			UIWidgetPair.Value->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HideAllUI Success."));
}

UUserWidget* UUIManagerComponent::GetUI(ELMSUIType UIType) const
{
	const TObjectPtr<UUserWidget>* FoundWidget = CreatedUIMap.Find(UIType);

	if (!FoundWidget)
	{
		return nullptr;
	}

	return FoundWidget->Get();
}

const FLMSUIData* UUIManagerComponent::FindUIData(ELMSUIType UIType) const
{
	for (const FLMSUIData& UIData : UIDataList)
	{
		if (UIData.UIType == UIType)
		{
			return &UIData;
		}
	}

	return nullptr;
}