// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_SelectAttackType.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SelectAttackType::UBTTask_SelectAttackType()
{
	NodeName = TEXT("Select Attack Pattern");


	PatternWeights.Add(EBossPattern::NormalAttack, 4.0f);
	PatternWeights.Add(EBossPattern::CounterStance, 1.0f);
}

EBTNodeResult::Type UBTTask_SelectAttackType::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;
    }

    // 가중치 합산
    float Total = 0.f;
    for (const TPair<EBossPattern, float>& Pair : PatternWeights)
    {
        if (Pair.Value > 0.f)
        {
            Total += Pair.Value;
        }
    }

    EBossPattern Picked = EBossPattern::NormalAttack;

    if (Total > 0.f)
    {
        const float Roll = FMath::FRandRange(0.f, Total);
        float Acc = 0.f;
        for (const TPair<EBossPattern, float>& Pair : PatternWeights)
        {
            if (Pair.Value <= 0.f) continue;

            Acc += Pair.Value;

            if (Roll <= Acc)
            {
                Picked = Pair.Key;
                break;
            }
        }
    }

    BB->SetValueAsEnum(TEXT("CurrentAttackType"), static_cast<uint8>(Picked));

    return EBTNodeResult::Succeeded;
}
