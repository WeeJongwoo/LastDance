// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LDEnemyCharacter.h"
#include "Component/LDStatComponent.h"
#include "Log/LDLog.h"
#include "AI/LDAIController.h"

ALDEnemyCharacter::ALDEnemyCharacter()
{
	SetReplicates(true);

	AIControllerClass = ALDAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ALDEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 스탯 초기화
	if (HasAuthority() && StatComponent)
	{
		SetupCharacterStats();
	}
}

void ALDEnemyCharacter::HandleDeath()
{
	Super::HandleDeath();

	// 5초 후 액터 제거 (필요에 따라 조정)
	SetLifeSpan(5.0f);

	LD_LOG(LDLog, Log, TEXT("Enemy Died: %s"), *GetName())
}
