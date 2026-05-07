// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SpawnHazard.generated.h"

class ALDHazardActor;
class ULDHazardDataAsset;

UCLASS()
class LASTDANCE_API UAnimNotify_SpawnHazard : public UAnimNotify
{
	GENERATED_BODY()
	
	
public:

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	TSubclassOf<ALDHazardActor> HazardClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	TObjectPtr<ULDHazardDataAsset> HazardData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	FName SpawnSocketName;   

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	FVector SpawnOffset = FVector::ZeroVector;
};
