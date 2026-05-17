// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LDMenuPlayerController.h"
#include "UI/LDMainMenuWidget.h"


void ALDMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    bShowMouseCursor = true;
    FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

    if (MainMenuWidgetClass)
    {
        MainMenuWidget = CreateWidget<ULDMainMenuWidget>(this, MainMenuWidgetClass);
        if (MainMenuWidget)
        {
            MainMenuWidget->AddToViewport();
        }
    }
}
