// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LDPlayerCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "Character/LDCharacterMovementComponent.h"

ULDPlayerCharacterAnimInstance::ULDPlayerCharacterAnimInstance()
{
	MovingThreshould = 3.f;
	JunpingThreshould = 100.f;
}

void ULDPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		ULDCharacterMovementComponent* MoveComp = Cast<ULDCharacterMovementComponent>(Owner->GetCharacterMovement());
		if (MoveComp)
		{
			Movement = MoveComp;
		}
	}
}

void ULDPlayerCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Movement)
	{
		Velocity = Movement->Velocity;
		GroundSpeed = Velocity.Size2D();
		bIsIdle = GroundSpeed < MovingThreshould;
		bIsFalling = Movement->IsFalling();
		bIsJumping = (Velocity.Z > JunpingThreshould) & bIsFalling;
		bIsRolling = Movement->IsRolling();

		if (bIsRolling && IsAnyMontagePlaying())
		{
			Montage_Stop(0.0f);
		}
	}
}
