// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LDBaseCharacter.h"
#include "Types/LDCharacterStats.h"
#include "LDEnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API ALDEnemyCharacter : public ALDBaseCharacter
{
	GENERATED_BODY()

public:
	ALDEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;

	// 에디터 디테일 패널에서 적마다 다르게 설정 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FLDCharacterStats DefaultStats;
	
};
