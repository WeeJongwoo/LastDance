// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AcatesAttack.generated.h"

class ALDBossCharacter;


UCLASS()
class LASTDANCE_API UBTTask_AcatesAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
	
public:

	UBTTask_AcatesAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	

protected:
	UFUNCTION()
	void AttackFinished();

private:
	TWeakObjectPtr<ALDBossCharacter> CachedBoss;
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
