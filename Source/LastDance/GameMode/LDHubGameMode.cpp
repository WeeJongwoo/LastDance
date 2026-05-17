// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LDHubGameMode.h"
#include "Player/LDPlayerController.h"
#include "Character/LDPlayerCharacter.h"

ALDHubGameMode::ALDHubGameMode()
{
	PlayerControllerClass = ALDPlayerController::StaticClass();
	DefaultPawnClass = ALDPlayerCharacter::StaticClass();
}

void ALDHubGameMode::BeginPlay()
{
	Super::BeginPlay();

	OnPlayerPossessed.AddDynamic(this, &ALDHubGameMode::PlayerPossessed);
}

void ALDHubGameMode::PlayerPossessed(ALDPlayerController* PC)
{
	if (!PC) return;

	if (ALDBaseCharacter* BaseChar = Cast<ALDBaseCharacter>(PC->GetPawn()))
	{
		BaseChar->OnDeath.AddUObject(this, &ALDHubGameMode::PlayerCharacterDied);
	}
}

void ALDHubGameMode::PlayerCharacterDied(ALDBaseCharacter* DeadCharacter)
{
	if (!DeadCharacter || !HasAuthority()) return;

	ALDPlayerController* PC = Cast<ALDPlayerController>(DeadCharacter->GetController());
	if (!PC) return;

	TWeakObjectPtr<ALDPlayerController> WeakPC(PC);
	TWeakObjectPtr<ALDBaseCharacter> WeakOldChar(DeadCharacter);

	GetWorldTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateLambda([WeakOldChar]()
			{
				if (auto* Old = WeakOldChar.Get())
				{
					Old->Destroy();
				}
			}));

	FTimerHandle Handle;
	FTimerDelegate Del;

	Del.BindUObject(this, &ALDHubGameMode::RespawnPlayer, WeakPC);
	GetWorldTimerManager().SetTimer(Handle, Del, RespawnDelay, false);
}

void ALDHubGameMode::RespawnPlayer(TWeakObjectPtr<ALDPlayerController> WeakPC)
{
	ALDPlayerController* PC = WeakPC.Get();
	if (!PC)
	{
		return;
	}

	// 기존 폰(시체) 제거
	if (APawn* OldPawn = PC->GetPawn())
	{
		OldPawn->Destroy();
	}

	RestartPlayer(PC);
}
