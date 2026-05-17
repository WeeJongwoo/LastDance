// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LDPlayerController.h"
#include "UI/LDHUDWidget.h"
#include "Interface/LDStatInterface.h"
#include "Component/LDStatComponent.h"
#include "Log/LDLog.h"
#include "GameMode/LDGameMode.h"
#include "Character/LDBossCharacter.h"
#include "UI/LDInGameMenuWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


ALDPlayerController::ALDPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bRegisteredWithGameMode = false;
}

void ALDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		if (HUDClass)
		{
			HUDWidget = CreateWidget<ULDHUDWidget>(this, HUDClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport();
				ApplyPendingBossToHUD();
			}
		}

		if (ControllerMappingContext)
		 {
			 if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			 {
				 Subsystem->AddMappingContext(ControllerMappingContext, 1);
			 }
		}
	}
}

void ALDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleMenuAction)
		{
			EnhancedInput->BindAction(ToggleMenuAction, ETriggerEvent::Started, this, &ALDPlayerController::ToggleInGameMenu);
		}
	}
}

void ALDPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority())
	{
		if (auto* GM = GetWorld()->GetAuthGameMode<ALDGameMode>())
		{
			GM->NotifyPlayerPossessed(this);
		}
	}
}

void ALDPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	if (HUDWidget)
	{
		HUDWidget->BindToStatComponent(P);
		HUDWidget->HideBossBar();
	}

	ApplyPendingBossToHUD();
}

void ALDPlayerController::UpdateHP(float NewHP, float MaxHP)
{
	LD_LOG(LDLog, Log, TEXT("Begin"));
	if (HUDWidget)
	{
		HUDWidget->UpdateHPBar(NewHP, MaxHP);
	}
}

void ALDPlayerController::ApplyPendingBossToHUD()
{
	if (HUDWidget && PendingBoss)
	{
		HUDWidget->ShowBossBar(PendingBoss);
		PendingBoss = nullptr;
	}
}

void ALDPlayerController::ClientRPC_NotifyRecognizedByBoss_Implementation(ALDBossCharacter* Boss)
{
	LD_LOG(LDLog, Log, TEXT("ClientRPC received. Boss=%s HUD=%s"), *GetNameSafe(Boss), HUDWidget ? TEXT("OK") : TEXT("NULL"));

	if (!Boss)
	{
		return;
	}

	if (HUDWidget)
	{
		HUDWidget->ShowBossBar(Boss);
	}
	else
	{
		PendingBoss = Boss;
		LD_LOG(LDLog, Log, TEXT("HUD not ready, pending boss: %s"), *GetNameSafe(Boss));
	}
}

void ALDPlayerController::ClientRPC_NotifyBossDefeated_Implementation(ALDBossCharacter* Boss)
{
	PendingBoss = nullptr;

	if (HUDWidget)
	{
		HUDWidget->HideBossBar();
	}
}

void ALDPlayerController::ToggleInGameMenu()
{
	if (!IsLocalController() || !InGameMenuClass) return;

	if (!InGameMenuWidget)
	{
		InGameMenuWidget = CreateWidget<ULDInGameMenuWidget>(this, InGameMenuClass);
		if (!InGameMenuWidget) return;

		InGameMenuWidget->AddToViewport(10);
		SetInGameMenuVisible(true);
		return;
	}

	const bool bCurrentlyVisible = (InGameMenuWidget->GetVisibility() != ESlateVisibility::Collapsed);
	SetInGameMenuVisible(!bCurrentlyVisible);
}

void ALDPlayerController::SetInGameMenuVisible(bool bVisible)
{
	if (!InGameMenuWidget) return;

	if (bVisible)
	{
		InGameMenuWidget->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(InGameMenuWidget->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
		bShowMouseCursor = true;

		if (APawn* P = GetPawn())
		{
			P->DisableInput(this);
		}
	}
	else
	{
		InGameMenuWidget->SetVisibility(ESlateVisibility::Collapsed);

		FInputModeGameOnly Mode;
		SetInputMode(Mode);
		bShowMouseCursor = false;

		if (APawn* P = GetPawn())
		{
			P->EnableInput(this);
		}
	}
}
