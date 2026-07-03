// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LDCharacterMovementComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRollStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRollFinished);

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_Rolling	UMETA(DisplayName = "Rolling"),
};


class FLDSavedMove_Character : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:
	virtual void Clear() override;

	virtual void SetInitialPosition(ACharacter* C) override;

	virtual uint8 GetCompressedFlags() const override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

	uint8 bPressedRolling : 1;
};

class FLDNetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;
public:

	FLDNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class LASTDANCE_API ULDCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	
public:
	ULDCharacterMovementComponent();

	void SetRollingCommand();

	bool IsRolling() const { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Rolling; }
	bool CanRoll() const;

	virtual float GetMaxSpeed() const override;

protected:
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void EnterRoll();
	void PhysRolling(float deltaTime, int32 Iterations);

public:

	uint8 bPressedRolling : 1;

	FOnRollStart OnRollStartDelegate;
	FOnRollFinished OnRollFinishedDelegate;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float RollDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float RollCooldown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float RollDistance;

	FVector RollDirection;
	float RollElapsedTime;
	float LastRollEndTime;
};
