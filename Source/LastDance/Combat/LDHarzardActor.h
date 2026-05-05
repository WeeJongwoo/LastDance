// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LDHarzardActor.generated.h"

class UShapeComponent;
class UDecalComponent;
class ULDHazardDataAsset;

UCLASS()
class LASTDANCE_API ALDHarzardActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALDHarzardActor();

    // 서버에서 SpawnActorDeferred 직후 호출
    void InitializeFromData(const ULDHazardDataAsset* InData);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    // 서브클래스용 가상 훅
    virtual bool ShouldDamageActor(AActor* Target) const;
    virtual void OnHazardActivated();
    virtual void OnHazardDestroyed();
	void StartApplyingDamage();

private:
    void ApplyDamageTick();
    void OnLifetimeExpired();

protected:

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY()
    TObjectPtr<UShapeComponent> OverlapShape;  // 동적 생성

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UDecalComponent> DecalComp;

    UPROPERTY(EditAnywhere, Category = "Hazard")
    TObjectPtr<const ULDHazardDataAsset> HazardData;

    FTimerHandle DamageTickHandle;
    FTimerHandle LifetimeHandle;
	FTimerHandle TelegraphHandle;
};
