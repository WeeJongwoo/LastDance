// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ULDHUDWidget;

/**
 * 
 */
UCLASS()
class LASTDANCE_API ALDPlayerController : public APlayerController
{
	GENERATED_BODY()
	

protected:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void AcknowledgePossession(class APawn* P) override;

	UFUNCTION()
	void UpdateHP(float NewHP, float MaxHP);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULDHUDWidget> HUDClass;


	TObjectPtr<ULDHUDWidget> HUDWidget;
};
