// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CounterStance.generated.h"

class ALDBossCharacter;

UCLASS()
class LASTDANCE_API UBTTask_CounterStance : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UBTTask_CounterStance();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UFUNCTION()
	void HandleStanceFinished(bool bTriggered);

private:
	TWeakObjectPtr<ALDBossCharacter> CachedBoss;
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

};
