// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LDNetworkSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "LastDance Network"))
class LASTDANCE_API ULDNetworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Network")
	FString ServerAddress = TEXT("127.0.0.1:7777");
	
};
