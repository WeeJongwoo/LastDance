// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SelectRandomTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/LDBossCharacter.h"
#include "Character/LDPlayerCharacter.h"

UBTTask_SelectRandomTarget::UBTTask_SelectRandomTarget()
{
}

EBTNodeResult::Type UBTTask_SelectRandomTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	ALDBossCharacter* Boss = Cast<ALDBossCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	TArray<ALDPlayerCharacter*> AliveTargets = Boss->GetAliveTargetPawns();

	if (AliveTargets.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	int32 RandomIndex = FMath::RandRange(0, AliveTargets.Num() - 1);

	ALDPlayerCharacter* SelectedTarget = AliveTargets[RandomIndex];

	OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("TargetPlayerPawn"), SelectedTarget);


	return EBTNodeResult::Succeeded;
}
