// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "../BaseEnemyCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABaseEnemyCharacter* Character = AIController ? Cast<ABaseEnemyCharacter>(AIController->GetPawn()) : nullptr;
	UAnimInstance* AnimInstance = Character ? Character->GetMesh()->GetAnimInstance() : nullptr;

	if (!Character)
	{
		return EBTNodeResult::Failed;
	}

	if (AbilityTag.IsValid())
	{
		if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
		{
			FGameplayTagContainer TagContainer(AbilityTag);
			ASC->TryActivateAbilitiesByTag(TagContainer);
		}
	}

	OwningComp = &OwnerComp;


	//Character->Destroy();

	return EBTNodeResult::Succeeded;
}

