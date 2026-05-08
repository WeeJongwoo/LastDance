// Fill out your copyright notice in the Description page of Project Settings.


#include "LDBossCharacter.h"
#include "LDPlayerCharacter.h"
#include "Player/LDPlayerController.h"
#include "Player/LDPlayerState.h"
#include "GameMode/LDGameMode.h"
#include "GameMode/LDGameState.h"
#include "Log/LDLog.h"

ALDBossCharacter::ALDBossCharacter()
{
}

void ALDBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (ALDGameState* GameState = World->GetGameState<ALDGameState>())
		{
			GameState->SetActiveBoss(this);

			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (!PS)
				{
					continue;
				}

				if (ALDPlayerController* PC = Cast<ALDPlayerController>(PS->GetPlayerController()))
				{
					RegisterController(PC);
				}
			}
		}
	}

	if (ALDGameMode* GameMode = GetWorld()->GetAuthGameMode<ALDGameMode>())
	{
		GameMode->OnPlayerPossessed.AddDynamic(this, &ALDBossCharacter::OnPlayerPossessedHandler);
		GameMode->OnPlayerLeft.AddDynamic(this, &ALDBossCharacter::OnPlayerLeftHandler);
	}
}

void ALDBossCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (ALDGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALDGameMode>() : nullptr)
		{
			GameMode->OnPlayerPossessed.RemoveDynamic(this, &ALDBossCharacter::OnPlayerPossessedHandler);
			GameMode->OnPlayerLeft.RemoveDynamic(this, &ALDBossCharacter::OnPlayerLeftHandler);
		}

		if (ALDGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ALDGameState>() : nullptr)
		{
			if (GameState->GetActiveBoss() == this)
			{
				GameState->SetActiveBoss(nullptr);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ALDBossCharacter::HandleDeath()
{
	if (HasAuthority())
	{
		for (TWeakObjectPtr<ALDPlayerController>& Weak : KnownControllers)
		{
			if (ALDPlayerController* PlayerController = Weak.Get())
			{
				PlayerController->ClientRPC_NotifyBossDefeated(this);
			}
		}
		KnownControllers.Reset();

		if (ALDGameState* GameState = GetWorld()->GetGameState<ALDGameState>())
		{
			if (GameState->GetActiveBoss() == this)
			{
				GameState->SetActiveBoss(nullptr);
			}
		}
	}

	Super::HandleDeath();
}

void ALDBossCharacter::RegisterController(ALDPlayerController* PC)
{
	if (!HasAuthority() || !PC)
	{
		return;
	}

	// 중복 검사
	for (const TWeakObjectPtr<ALDPlayerController>& Weak : KnownControllers)
	{
		if (Weak.Get() == PC)
		{
			return;
		}
	}

	KnownControllers.Add(PC);

	LD_LOG(LDLog, Log, TEXT("Boss registered PC: %s (total=%d)"), *GetNameSafe(PC), KnownControllers.Num());

	PC->ClientRPC_NotifyRecognizedByBoss(this);
}

void ALDBossCharacter::UnregisterController(ALDPlayerController* PC)
{
	if (!HasAuthority() || !PC)
	{
		return;
	}

	const int32 Removed = KnownControllers.RemoveAll([PC](const TWeakObjectPtr<ALDPlayerController>& W)
		{
			return !W.IsValid() || W.Get() == PC;
		});

	if (Removed > 0)
	{
		LD_LOG(LDLog, Log, TEXT("Boss unregistered PC: %s (total=%d)"), *GetNameSafe(PC), KnownControllers.Num());
	}
}

TArray<ALDPlayerCharacter*> ALDBossCharacter::GetAliveTargetPawns() const
{
	TArray<ALDPlayerCharacter*> Out;
	Out.Reserve(KnownControllers.Num());

	for (const TWeakObjectPtr<ALDPlayerController>& Weak : KnownControllers)
	{
		ALDPlayerController* PC = Weak.Get();
		if (!PC) continue;
		if (ALDPlayerCharacter* P = PC->GetPawn<ALDPlayerCharacter>())
		{
			if (!P->IsDead())
			{
				Out.Add(P);
			}
		}
	}
	return Out;
}

void ALDBossCharacter::OnPlayerPossessedHandler(ALDPlayerController* PC)
{
	RegisterController(PC);
}

void ALDBossCharacter::OnPlayerLeftHandler(ALDPlayerController* PC)
{
	UnregisterController(PC);
}
