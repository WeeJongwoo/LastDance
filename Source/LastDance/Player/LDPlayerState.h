// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/LDCharacterStatDataAsset.h"
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

	const FLDCharacterStats& GetBaseStats() const { return BaseStatData->CharacterStats; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated, EditAnywhere)
	TObjectPtr<ULDCharacterStatDataAsset> BaseStatData;
	
	
};
