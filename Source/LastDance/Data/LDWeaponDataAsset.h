// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/LDCharacterStats.h"
#include "LDWeaponDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class LASTDANCE_API ULDWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(EditAnywhere, Category = "Weapon Stats")
	FLDWeaponStats WeaponStats;
	
};
