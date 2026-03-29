// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LDPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ALDPlayerCharacter::ALDPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}


void ALDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ALDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

