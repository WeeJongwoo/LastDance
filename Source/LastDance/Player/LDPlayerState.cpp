// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LDPlayerState.h"
#include "Net/UnrealNetwork.h"

ALDPlayerState::ALDPlayerState()
{
}

void ALDPlayerState::SetBaseStats(const FLDCharacterStats& InStats)
{
	if (HasAuthority())
	{
		BaseStats = InStats;
	}
}

void ALDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ALDPlayerState, BaseStats, COND_OwnerOnly);
}
