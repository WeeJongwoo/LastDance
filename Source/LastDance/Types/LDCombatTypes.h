// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LDCombatTypes.generated.h"


UENUM(BlueprintType)
enum class EWeaponTraceType : uint8
{
	Line        UMETA(DisplayName = "Line Trace (얇은 검, 도)"),
	Sphere      UMETA(DisplayName = "Sphere Sweep (일반 무기)"),
	Box         UMETA(DisplayName = "Box Sweep (대검, 망치)")
};

USTRUCT()
struct FWeaponTraceSample
{
	GENERATED_BODY()

	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector EndLocation = FVector::ZeroVector;
};

USTRUCT()
struct FWeaponTraceParams
{
	GENERATED_BODY()

	UPROPERTY()
	EWeaponTraceType TraceType = EWeaponTraceType::Sphere;

	UPROPERTY()
	float SphereRadius = 5.0f;

	UPROPERTY()
	FVector BoxHalfSize = FVector(10.0f, 10.0f, 10.0f);

	UPROPERTY()
	float Damage = 10.0f;

	UPROPERTY()
	bool bUseBladeSurface = true;

	UPROPERTY()
	bool bShowDebugTrace = false;
};
