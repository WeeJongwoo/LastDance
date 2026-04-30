// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LDPlayerState.h"
#include "Net/UnrealNetwork.h"

ALDPlayerState::ALDPlayerState()
{
}

void ALDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ALDPlayerState, BaseStatData, COND_OwnerOnly);
}
