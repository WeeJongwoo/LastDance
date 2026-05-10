// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_CounterStance.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/LDBossCharacter.h"


UBTTask_CounterStance::UBTTask_CounterStance()
{
    NodeName = TEXT("Counter Stance");
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_CounterStance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        return EBTNodeResult::Failed;
    }

    ALDBossCharacter* Boss = Cast<ALDBossCharacter>(AIC->GetPawn());
    if (!Boss)
    {
        return EBTNodeResult::Failed;
    }

    CachedBoss = Boss;
    CachedOwnerComp = &OwnerComp;

    Boss->OnCounterStanceFinished.AddDynamic(this, &UBTTask_CounterStance::HandleStanceFinished);

    Boss->EnterCounterStance();

    if (!Boss->IsInCounterStance())
    {
        Boss->OnCounterStanceFinished.RemoveDynamic(this, &UBTTask_CounterStance::HandleStanceFinished);
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_CounterStance::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ALDBossCharacter* Boss = CachedBoss.Get())
    {
        Boss->OnCounterStanceFinished.RemoveDynamic(this, &UBTTask_CounterStance::HandleStanceFinished);
        if (Boss->IsCounterSequenceActive())
        {
            Boss->ExitCounterStance(false);
        }
    }

    CachedBoss.Reset();
    CachedOwnerComp.Reset();

    return EBTNodeResult::Aborted;
}

void UBTTask_CounterStance::HandleStanceFinished(bool bTriggered)
{
    if (ALDBossCharacter* Boss = CachedBoss.Get())
    {
        Boss->OnCounterStanceFinished.RemoveDynamic(this, &UBTTask_CounterStance::HandleStanceFinished);
    }

    UBehaviorTreeComponent* OC = CachedOwnerComp.Get();
    CachedBoss.Reset();
    CachedOwnerComp.Reset();

    if (OC)
    {
        FinishLatentTask(*OC, EBTNodeResult::Succeeded);
    }
}
