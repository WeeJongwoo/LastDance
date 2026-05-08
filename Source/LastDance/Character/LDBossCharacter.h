// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LDEnemyCharacter.h"
#include "LDBossCharacter.generated.h"

class ALDPlayerController;
class ALDPlayerCharacter;

UCLASS()
class LASTDANCE_API ALDBossCharacter : public ALDEnemyCharacter
{
	GENERATED_BODY()
	
	
public:
	ALDBossCharacter();
	
	void RegisterController(ALDPlayerController* PC);
	void UnregisterController(ALDPlayerController* PC);

	TArray<ALDPlayerCharacter*> GetAliveTargetPawns() const;


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleDeath() override;

	UFUNCTION()
	void OnPlayerPossessedHandler(ALDPlayerController* PC);

	UFUNCTION()
	void OnPlayerLeftHandler(ALDPlayerController* PC);

protected:

	UPROPERTY()
	TArray<TWeakObjectPtr<ALDPlayerController>> KnownControllers;
};
