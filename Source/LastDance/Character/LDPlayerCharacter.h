// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LDBaseCharacter.h"
#include "LDPlayerCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;

/**
 * 
 */
UCLASS()
class LASTDANCE_API ALDPlayerCharacter : public ALDBaseCharacter
{
	GENERATED_BODY()
	

	
public:
	ALDPlayerCharacter();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
};
