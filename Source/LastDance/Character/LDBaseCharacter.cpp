// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LDBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Component/LDStatComponent.h"
#include "Log/LDLog.h"

// Sets default values
ALDBaseCharacter::ALDBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Player"));

	CombatComponent = CreateDefaultSubobject<ULDCombatComponent>(TEXT("CombatComponent"));
	StatComponent = CreateDefaultSubobject<ULDStatComponent>(TEXT("StatComponent"));
}

// Called when the game starts or when spawned
void ALDBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALDBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALDBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

ULDCombatComponent* ALDBaseCharacter::GetCombatComponent() const
{
	return CombatComponent;
}

float ALDBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (StatComponent && !StatComponent->IsDead())
	{
		ActualDamage = StatComponent->ApplyDamage(ActualDamage);

		if (StatComponent->IsDead())
		{
			HandleDeath();
		}
	}

	return ActualDamage;
}

void ALDBaseCharacter::HandleDeath()
{
	LD_LOG(LDLog, Log, TEXT("Character Died: %s"), *GetName());

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	if (AController* MyController = GetController())
	{
		MyController->UnPossess();
	}

	MulticastRPC_HandleDeath();
}


void ALDBaseCharacter::MulticastRPC_HandleDeath_Implementation()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
}



