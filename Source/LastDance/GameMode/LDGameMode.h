// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LDGameMode.generated.h"


class ALDPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerPossessed, ALDPlayerController*, PC);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, ALDPlayerController*, PC);

UCLASS()
class LASTDANCE_API ALDGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	
public:
	ALDGameMode();

	// PC가 Possess 시 자기 자신을 인자로 호출. 서버 전용.
	void NotifyPlayerPossessed(ALDPlayerController* PC);

	// 진짜로 떠났을 때 (디스커넥트 등)
	virtual void Logout(AController* Exiting) override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerPossessed OnPlayerPossessed;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerLeft OnPlayerLeft;
	
};
