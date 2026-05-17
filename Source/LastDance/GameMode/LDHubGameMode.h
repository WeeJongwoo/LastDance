// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/LDGameMode.h"
#include "LDHubGameMode.generated.h"

class ALDBaseCharacter;

UCLASS()
class LASTDANCE_API ALDHubGameMode : public ALDGameMode
{
	GENERATED_BODY()
	
public:

	ALDHubGameMode();
	
protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void PlayerPossessed(ALDPlayerController* PC);

	void RespawnPlayer(TWeakObjectPtr<ALDPlayerController> WeakPC);

	void PlayerCharacterDied(ALDBaseCharacter* DeadCharacter);

protected:
	float RespawnDelay = 2.0f;
};
