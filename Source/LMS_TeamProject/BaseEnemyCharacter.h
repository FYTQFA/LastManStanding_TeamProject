// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyTableRow.h"
#include "BaseEnemyCharacter.generated.h"

UCLASS()
class LMS_TEAMPROJECT_API ABaseEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	int UniqueID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	TObjectPtr<class UAnimMontage> AttackMontage;

	/** Looks up this enemy's row via UDataTableSubSystem using UniqueID. Returns nullptr if not found. */
	const FEnemyTableRow* GetEnemyData() const;
};
