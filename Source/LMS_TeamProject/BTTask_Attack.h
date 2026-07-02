// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Attack.generated.h"

/**
 * Plays an attack montage on the controlled character and finishes the task
 * (Succeeded/Failed) once the montage ends.
 */
UCLASS()
class LMS_TEAMPROJECT_API UBTTask_Attack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_Attack();

	/** Montage to play when this task executes */

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	TWeakObjectPtr<UBehaviorTreeComponent> OwningComp;
};
