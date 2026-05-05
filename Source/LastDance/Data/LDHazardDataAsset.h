// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LDHazardDataAsset.generated.h"

class UNiagaraSystem;
class UMaterialInterface;

UENUM(BlueprintType)
enum class ELDHazardShapeType : uint8
{
	Sphere,
	Box,
	
};

UCLASS()
class LASTDANCE_API ULDHazardDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	ELDHazardShapeType HazardShapeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard",
		meta = (EditCondition = "HazardShapeType == ELDHazardShapeType::Sphere", EditConditionHides))
	float SphereRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard",
		meta = (EditCondition = "HazardShapeType == ELDHazardShapeType::Box", EditConditionHides))
	FVector BoxExtent = FVector(200, 200, 100);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float TelegraphDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float Lifetime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float TickInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float DamagePerTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	TSoftObjectPtr<UNiagaraSystem> VisualFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	TSoftObjectPtr<UMaterialInterface> GroundDecal;

};
