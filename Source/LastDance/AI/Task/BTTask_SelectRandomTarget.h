// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SelectRandomTarget.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API UBTTask_SelectRandomTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SelectRandomTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
