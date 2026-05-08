// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LDGameMode.h"
#include "Player/LDPlayerController.h"
#include "Log/LDLog.h"

ALDGameMode::ALDGameMode()
{
}

void ALDGameMode::NotifyPlayerPossessed(ALDPlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	OnPlayerPossessed.Broadcast(PC);
	LD_LOG(LDLog, Log, TEXT("Possessed by PC: %s"), *GetNameSafe(PC));
}

void ALDGameMode::Logout(AController* Exiting)
{
	if (auto* PC = Cast<ALDPlayerController>(Exiting))
	{
		OnPlayerLeft.Broadcast(PC);
		LD_LOG(LDLog, Log, TEXT("Logout by PC: %s"), *GetNameSafe(PC));
	}

	Super::Logout(Exiting);
}


