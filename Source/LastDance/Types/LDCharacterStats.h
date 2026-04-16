#pragma once

#include "CoreMinimal.h"
#include "LDCharacterStats.generated.h"

USTRUCT(BlueprintType)
struct FLDCharacterStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ATK = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DEF = 5.0f;
};

USTRUCT(BlueprintType)
struct FLDWeaponStats
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeaponATK = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeaponDEF = 0.0f;
};