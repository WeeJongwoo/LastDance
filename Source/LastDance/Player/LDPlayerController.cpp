// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LDPlayerController.h"
#include "UI/LDHUDWidget.h"
#include "Interface/LDStatInterface.h"
#include "Component/LDStatComponent.h"
#include "Log/LDLog.h"

void ALDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		if (HUDClass)
		{
			HUDWidget = CreateWidget<ULDHUDWidget>(this, HUDClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport();
			}
		}
	}
}

void ALDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ALDPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ALDPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	if (HUDWidget)
	{
		HUDWidget->BindToStatComponent(P);
	}
}

void ALDPlayerController::UpdateHP(float NewHP, float MaxHP)
{
	LD_LOG(LDLog, Log, TEXT("Begin"));
	if (HUDWidget)
	{
		HUDWidget->UpdateHPBar(NewHP, MaxHP);
	}
}
