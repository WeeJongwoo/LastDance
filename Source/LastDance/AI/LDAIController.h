// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LDAIController.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API ALDAIController : public AAIController
{
	GENERATED_BODY()
	
	
public:
	ALDAIController();

	void RunAI();
	void StopAI();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<class UBlackboardData> BBAsset;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<class UBehaviorTree> BTAsset;
	
};
