// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LDPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Log/LDLog.h"
#include "Player/LDPlayerState.h"
#include "Component/LDStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "LDCharacterMovementComponent.h"
#include "Component/LDAttackComboComponent.h"

ALDPlayerCharacter::ALDPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<ULDCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	AttackComboComponent = CreateDefaultSubobject<ULDAttackComboComponent>(TEXT("AttackComboComponent"));

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
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ALDPlayerCharacter::Roll);

		for (const FLDAttackInputBinding& Binding : AttackInputBindings)
		{
			if (Binding.Action)
			{
				// 마지막 인자(Binding.Command)는 호출될 때 OnAttackCommand의 인자로 전달됨
				EnhancedInputComponent->BindAction(Binding.Action, ETriggerEvent::Started, this, &ALDPlayerCharacter::OnAttackCommand, Binding.Command);
			}
		}
	}
}

void ALDPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetupCharacterStats();
}

void ALDPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (!IsValid(Controller))
	{
		return;
	}

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

void ALDPlayerCharacter::OnAttackCommand(ELDAttackCommand Command)
{
	if (AttackComboComponent)
	{
		AttackComboComponent->HandleCommand(Command);
	}
}

void ALDPlayerCharacter::Roll(const FInputActionValue& Value)
{
	ULDCharacterMovementComponent* LDMovement = Cast<ULDCharacterMovementComponent>(GetCharacterMovement());
	if (LDMovement)
	{
		LDMovement->SetRollingCommand();
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

void ALDPlayerCharacter::SetupCharacterStats()
{

	if (ALDPlayerState* PS = GetPlayerState<ALDPlayerState>())
	{
		if (StatComponent)
		{
			StatComponent->InitializeStats(PS->GetBaseStats());
		}
	}
}

void ALDPlayerCharacter::HandleDeath()
{
	GetMesh()->SetVisibility(false);

	Super::HandleDeath();
}

float ALDPlayerCharacter::GetAttackDamage() const
{
	const float Multiplier = AttackComboComponent ? AttackComboComponent->GetCurrentDamageMultiplier() : 1.0f;
	return Super::GetAttackDamage() * Multiplier;
}