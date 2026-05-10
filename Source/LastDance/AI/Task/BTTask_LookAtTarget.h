// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LookAtTarget.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API UBTTask_LookAtTarget : public UBTTaskNode
{
	GENERATED_BODY()
	

public:
	UBTTask_LookAtTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

	UPROPERTY(EditAnywhere)
	float RotationSpeed;
	
	
};
