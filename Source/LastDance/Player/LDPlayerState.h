// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Types/LDCharacterStats.h"
#include "LDPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API ALDPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ALDPlayerState();

	void SetBaseStats(const FLDCharacterStats& InStats);
	const FLDCharacterStats& GetBaseStats() const { return BaseStats; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated, EditAnywhere)
	FLDCharacterStats BaseStats;
	
	
};
