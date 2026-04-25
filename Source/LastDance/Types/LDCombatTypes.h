// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LDCombatTypes.generated.h"

UENUM(BlueprintType)
enum class ETraceChannelType : uint8
{
	Player		UMETA(DisplayName = "플레이어가 공격"),
	Enemy		UMETA(DisplayName = "적이 공격"),
	Any			UMETA(DisplayName = "히트 대상 아무나")
};


UENUM(BlueprintType)
enum class EAttackTraceType : uint8
{
	Line        UMETA(DisplayName = "Line Trace (얇은 검, 도)"),
	Sphere      UMETA(DisplayName = "Sphere Sweep (일반 무기)"),
	Box         UMETA(DisplayName = "Box Sweep (대검, 망치)")
};

USTRUCT()
struct FAttackTraceSample
{
	GENERATED_BODY()

	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector EndLocation = FVector::ZeroVector;
};

USTRUCT()
struct FAttackTraceParams
{
	GENERATED_BODY()

	UPROPERTY()
	EAttackTraceType TraceType = EAttackTraceType::Sphere;

	UPROPERTY()
	float SphereRadius = 5.0f;

	UPROPERTY()
	FVector BoxHalfSize = FVector(10.0f, 10.0f, 10.0f);

	UPROPERTY()
	bool bUseBladeSurface = false;

	UPROPERTY()
	bool bShowDebugTrace = false;
};

