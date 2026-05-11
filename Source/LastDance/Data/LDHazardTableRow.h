#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "LDHazardDataAsset.h"
#include "LDHazardTableRow.generated.h"

USTRUCT(BlueprintType)
struct FLDHazardTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    ELDHazardShapeType HazardShapeType = ELDHazardShapeType::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard", meta = (EditCondition = "HazardShapeType == ELDHazardShapeType::Sphere"))
    float SphereRadius = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard", meta = (EditCondition = "HazardShapeType == ELDHazardShapeType::Box"))
    FVector BoxExtent = FVector(200.f, 200.f, 100.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    float TelegraphDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    float Lifetime = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    float TickInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    float DamagePerTick = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    TSoftObjectPtr<UNiagaraSystem> VisualFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    TObjectPtr<UMaterialInterface> GroundDecal;
};