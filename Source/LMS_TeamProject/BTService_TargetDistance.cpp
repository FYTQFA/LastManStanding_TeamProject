// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TargetDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_TargetDistance::UBTService_TargetDistance()
{
	NodeName = TEXT("Target Distance");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UBTService_TargetDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!ControlledPawn || !BlackboardComp)
	{
		return;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (!TargetActor)
	{
		return;
	}

	const float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
	const bool bInRange = Distance <= DistanceThreshold;
	
	BlackboardComp->SetValueAsBool(GetSelectedBlackboardKey(), bInRange);
}

