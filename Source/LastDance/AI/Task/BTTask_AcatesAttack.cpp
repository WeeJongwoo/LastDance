// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_AcatesAttack.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/LDBossCharacter.h"

UBTTask_AcatesAttack::UBTTask_AcatesAttack()
{
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_AcatesAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    Boss->OnAttackMontageEnded.AddDynamic(this, &UBTTask_AcatesAttack::AttackFinished);

    Boss->PlayAttackMontage();

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_AcatesAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ALDBossCharacter* Boss = CachedBoss.Get())
    {
        Boss->OnAttackMontageEnded.RemoveDynamic(this, &UBTTask_AcatesAttack::AttackFinished);
    }

    CachedBoss.Reset();
    CachedOwnerComp.Reset();

	return EBTNodeResult::Aborted;
}

void UBTTask_AcatesAttack::AttackFinished()
{
    if (ALDBossCharacter* Boss = CachedBoss.Get())
    {
        Boss->OnAttackMontageEnded.RemoveDynamic(this, &UBTTask_AcatesAttack::AttackFinished);
    }

    UBehaviorTreeComponent* OC = CachedOwnerComp.Get();
    CachedBoss.Reset();
    CachedOwnerComp.Reset();

    if (OC)
    {
        FinishLatentTask(*OC, EBTNodeResult::Succeeded);
    }
}
