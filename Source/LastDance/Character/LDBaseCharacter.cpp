// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LDBaseCharacter.h"


// Sets default values
ALDBaseCharacter::ALDBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

