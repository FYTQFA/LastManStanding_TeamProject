// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "Engine/GameInstance.h"
#include "DataTableSubSystem.h"

// Sets default values
ABaseEnemyCharacter::ABaseEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
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


