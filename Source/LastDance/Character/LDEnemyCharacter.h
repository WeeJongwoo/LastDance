// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LDBaseCharacter.h"
#include "Data/LDCharacterStatDataAsset.h"
#include "LDEnemyCharacter.generated.h"

UCLASS()
class LASTDANCE_API ALDEnemyCharacter : public ALDBaseCharacter
{
	GENERATED_BODY()

public:
	ALDEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;

};
