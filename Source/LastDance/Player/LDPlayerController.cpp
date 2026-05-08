// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LDPlayerController.h"
#include "UI/LDHUDWidget.h"
#include "Interface/LDStatInterface.h"
#include "Component/LDStatComponent.h"
#include "Log/LDLog.h"
#include "GameMode/LDGameMode.h"
#include "GameMode/LDGameState.h"
#include "Character/LDBossCharacter.h"


ALDPlayerController::ALDPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bRegisteredWithGameMode = false;
}

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
				ApplyPendingBossToHUD();
			}
		}
	}

	if (auto* GS = GetWorld()->GetGameState<ALDGameState>())
	{
		LD_LOG(LDLog, Log, TEXT("GameState OK: %s"), *GS->GetName());
	}
	else
	{
		LD_LOG(LDLog, Warning, TEXT("GameState cast FAILED — GameModeClass 확인 필요"));
	}
}

void ALDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ALDPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority() && !bRegisteredWithGameMode)
	{
		if (auto* GM = GetWorld()->GetAuthGameMode<ALDGameMode>())
		{
			bRegisteredWithGameMode = true;
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
