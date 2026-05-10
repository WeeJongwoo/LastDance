// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AI/LDBossPattern.h"
#include "BTTask_SelectAttackType.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API UBTTask_SelectAttackType : public UBTTaskNode
{
	GENERATED_BODY()
	
	
public:
	UBTTask_SelectAttackType();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Pattern")
	TMap<EBossPattern, float> PatternWeights;
	
};
