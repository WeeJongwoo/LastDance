// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LDMenuPlayerController.generated.h"

class ULDMainMenuWidget;

UCLASS()
class LASTDANCE_API ALDMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	
	virtual void BeginPlay() override;

	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ULDMainMenuWidget> MainMenuWidgetClass;
	

	TObjectPtr<ULDMainMenuWidget> MainMenuWidget;
};
