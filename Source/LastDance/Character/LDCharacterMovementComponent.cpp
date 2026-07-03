// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LDCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Log/LDLog.h"

ULDCharacterMovementComponent::ULDCharacterMovementComponent()
{
	bPressedRolling = false;

	RollDistance = 400.0f;
	RollCooldown = 1.0f;
	RollDuration = 0.3f;
}

void ULDCharacterMovementComponent::SetRollingCommand()
{
	bPressedRolling = true;
}

bool ULDCharacterMovementComponent::CanRoll() const
{
	if (IsRolling())
	{
		return false;
	}

	if (!IsMovingOnGround())
	{
		return false;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	return (Now - LastRollEndTime) >= RollCooldown;
}

float ULDCharacterMovementComponent::GetMaxSpeed() const
{
	if (IsRolling())
	{
		return RollDistance / RollDuration;
	}

	return Super::GetMaxSpeed();
}

FNetworkPredictionData_Client* ULDCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		ULDCharacterMovementComponent* MutableThis = const_cast<ULDCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FLDNetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}


void ULDCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (bPressedRolling)
	{
		if (CanRoll())
		{
			EnterRoll();
		}
		
		bPressedRolling = false;
	}
}

void ULDCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bPressedRolling = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void ULDCharacterMovementComponent::EnterRoll()
{
	FVector Dir = Acceleration.GetSafeNormal2D();
	if (Dir.IsNearlyZero())
	{
		Dir = Velocity.GetSafeNormal2D();
	}
	if (Dir.IsNearlyZero() && CharacterOwner)
	{
		Dir = CharacterOwner->GetActorForwardVector().GetSafeNormal2D();
	}

	RollDirection = Dir;
	RollElapsedTime = 0.0f;

	// 구르는 방향으로 즉시 회전 (소울류)
	if (CharacterOwner)
	{
		CharacterOwner->SetActorRotation(FRotator(0.0f, Dir.Rotation().Yaw, 0.0f));
	}

	SetMovementMode(MOVE_Custom, CMOVE_Rolling);
}


void ULDCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Rolling:
		PhysRolling(deltaTime, Iterations);
		break;
	default:
		break;
	}
}

void ULDCharacterMovementComponent::PhysRolling(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RollElapsedTime += deltaTime;

	Velocity = RollDirection * (RollDistance / RollDuration);

	const FVector Delta = Velocity * deltaTime;
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		// 벽에 막히면 밀어붙이지 않고 표면을 따라 미끄러짐
		SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}

	// 바닥 유지 (경사/계단에서 뜨는 것 방지)
	FFindFloorResult Floor;
	FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);
	if (Floor.IsWalkableFloor())
	{
		const float FloorDist = Floor.GetDistanceToFloor();
		if (FloorDist > KINDA_SMALL_NUMBER)
		{
			FHitResult FloorHit;
			SafeMoveUpdatedComponent(FVector(0.0f, 0.0f, -FloorDist), UpdatedComponent->GetComponentQuat(), true, FloorHit);
		}
	}
	else
	{
		// 발밑이 사라지면 낙하로 전환, 남은 시간을 낙하 물리에 넘김
		SetMovementMode(MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	if (RollElapsedTime >= RollDuration)
	{
		SetMovementMode(MOVE_Walking);
	}
}


void ULDCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (CharacterOwner && CharacterOwner->bClientUpdating)
	{
		return;
	}

	const bool bWasRolling = (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Rolling);
	const bool bIsNowRolling = IsRolling();

	if (!bWasRolling && bIsNowRolling)
	{
		OnRollStartDelegate.Broadcast();
	}
	else if (bWasRolling && !bIsNowRolling)
	{
		LastRollEndTime = GetWorld()->GetTimeSeconds();
		OnRollFinishedDelegate.Broadcast();
	}
}

FLDNetworkPredictionData_Client_Character::FLDNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FLDNetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new FLDSavedMove_Character());
}

void FLDSavedMove_Character::Clear()
{
	Super::Clear();

	bPressedRolling = false;
}

void FLDSavedMove_Character::SetInitialPosition(ACharacter* C)
{
	Super::SetInitialPosition(C);

	ULDCharacterMovementComponent* LDMovement = Cast<ULDCharacterMovementComponent>(C->GetCharacterMovement());
	if (LDMovement)
	{
		bPressedRolling = LDMovement->bPressedRolling;
	}
}

uint8 FLDSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bPressedRolling)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

bool FLDSavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FLDSavedMove_Character* NewLDMove = static_cast<const FLDSavedMove_Character*>(NewMove.Get());

	if (bPressedRolling != NewLDMove->bPressedRolling)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}
