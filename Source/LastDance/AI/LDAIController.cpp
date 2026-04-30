// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LDAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Log/LDLog.h"

ALDAIController::ALDAIController()
{
	bAllowTickBeforeBeginPlay = true;
}

void ALDAIController::RunAI()
{
	if (!IsValid(BBAsset) || !IsValid(BTAsset))
	{
		LD_LOG(LDLog, Warning, TEXT("Invalid Blackboard or Behavior Tree asset"));
		return;
	}

	UBlackboardComponent* BlackboradPtr = Blackboard.Get();
	if (UseBlackboard(BBAsset, BlackboradPtr))
	{

		bool RunResult = RunBehaviorTree(BTAsset);
		ensure(RunResult);
	}
}

void ALDAIController::StopAI()
{
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}
}

void ALDAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ALDAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	RunAI();
}
