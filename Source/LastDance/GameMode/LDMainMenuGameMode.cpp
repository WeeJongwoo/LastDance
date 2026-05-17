// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LDMainMenuGameMode.h"
#include "Player/LDMenuPlayerController.h"

ALDMainMenuGameMode::ALDMainMenuGameMode()
{
	PlayerControllerClass = ALDMenuPlayerController::StaticClass();
}
