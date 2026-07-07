// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "LMSAttributeSet.h"
#include "Engine/GameInstance.h"
#include "DataTableSubSystem.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<ULMSAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	const FEnemyTableRow* Data = GetEnemyData();
	if (Data)
	{
		AbilitySystemComponent->ApplyModToAttribute(
			ULMSAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			Data->MaxHP);
	}
}

void ABaseEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		GiveDefaultAbilities();
	}
}

void ABaseEnemyCharacter::GiveDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	for (TSubclassOf<UGameplayAbility>& Ability : DefaultAbilities)
	{
		if (Ability)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1));
		}
	}
}

void ABaseEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

const FEnemyTableRow* ABaseEnemyCharacter::GetEnemyData() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UDataTableSubSystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UDataTableSubSystem>() : nullptr;
	return DataSubsystem ? DataSubsystem->GetEnemyData(UniqueID) : nullptr;
}


