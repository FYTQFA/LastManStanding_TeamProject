// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BaseEnemyCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

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
	const float PlayLength = Character->PlayAnimMontage(Character->AttackMontage);
	if (PlayLength <= 0.f)
	{
		return EBTNodeResult::Failed;
	}

	OwningComp = &OwnerComp;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UBTTask_Attack::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Character->AttackMontage);

	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (UBehaviorTreeComponent* BTComp = OwningComp.Get())
	{
		FinishLatentTask(*BTComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}

