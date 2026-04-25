// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LDPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Log/LDLog.h"
#include "Player/LDPlayerState.h"
#include "Component/LDStatComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Components/CapsuleComponent.h"

ALDPlayerCharacter::ALDPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	bCanAttack = true;

	SetReplicates(true);
}


void ALDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	EnableInput(PlayerController);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

}

void ALDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALDPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALDPlayerCharacter::Look);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ALDPlayerCharacter::Attack);
	}
}

void ALDPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALDPlayerCharacter, bCanAttack);
}

void ALDPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ALDPlayerState* PS = GetPlayerState<ALDPlayerState>())
	{
		if (StatComponent)
		{
			StatComponent->InitializeStats(PS->GetBaseStats());
		}
	}

}

void ALDPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ALDPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ALDPlayerCharacter::Attack(const FInputActionValue& Value)
{
	if (bCanAttack)
	{
		bCanAttack = false;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

		PlayAttackMontage(0.0f);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindWeakLambda(this, [this](UAnimMontage* Montage, bool bInterrupted)
				{
					LD_LOG(LDLog, Log, TEXT("Attack montage ended"));

					if (Montage == AttackMontage)
					{
						bCanAttack = true;
						GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
						ServerRPC_AttackEnd();
					}
				});

			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AttackMontage);
		}

		ServerRPC_Attack();
	}
}

void ALDPlayerCharacter::OnAttackHit(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || !StatComponent) return;

	// 거리 계산 (캡슐 반경 보정)
	float Distance;

	if (ACharacter* HitChar = Cast<ACharacter>(HitActor))
	{
		float CapsuleR = HitChar->GetCapsuleComponent()->GetScaledCapsuleRadius();
		Distance = FVector::Distance(GetActorLocation(), HitChar->GetActorLocation()) - CapsuleR;
	}
	else
	{
		Distance = FVector::Distance(GetActorLocation(), HitResult.ImpactPoint);
	}

	if (Distance > StatComponent->GetAttackRange() * 1.2f)
	{
		return;
	}

#if ENABLE_DRAW_DEBUG
	if (UWorld* World = GetWorld())
	{
		DrawDebugSphere(World, HitResult.ImpactPoint, 15.0f, 12, FColor::Green, false, 2.0f);
	}
#endif

	// 사망 체크 + 데미지는 베이스 호출로 위임
	Super::OnAttackHit(HitResult);

	LD_LOG(LDLog, Log, TEXT("Weapon Hit: %s (ATK: %.1f)"), *HitActor->GetName(), StatComponent->GetATK());
}

void ALDPlayerCharacter::PlayAttackMontage(float TimeDiff)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && IsValid(AttackMontage))
	{
		if (AttackMontage->GetPlayLength() > TimeDiff)
		{
			AnimInstance->StopAllMontages(0.0f);
			AnimInstance->Montage_Play(AttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, TimeDiff);
		}

	}
}

void ALDPlayerCharacter::OnRep_CanAttack()
{
	if (!bCanAttack)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}

bool ALDPlayerCharacter::ServerRPC_Attack_Validate()
{
	return bCanAttack;
}


void ALDPlayerCharacter::ServerRPC_Attack_Implementation()
{
	if (!bCanAttack)
	{
		return;
	}

	bCanAttack = false;
	OnRep_CanAttack();

	float ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	MulticastRPC_PlayAttackMontage(ServerTime);
}


void ALDPlayerCharacter::ServerRPC_AttackEnd_Implementation()
{
	bCanAttack = true;
	OnRep_CanAttack();
}

void ALDPlayerCharacter::MulticastRPC_PlayAttackMontage_Implementation(float ServerStartTime)
{
	if (!IsLocallyControlled())
	{
		float LocalTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		float TimeDiff = LocalTime - ServerStartTime;

		PlayAttackMontage(TimeDiff);
	}
}
