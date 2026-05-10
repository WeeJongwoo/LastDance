// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckTargetInForward.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API UBTService_CheckTargetInForward : public UBTService
{
	GENERATED_BODY()
	
	
public:
	UBTService_CheckTargetInForward();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:

	UPROPERTY(EditAnywhere)
	float FOVAngle;
	
};
