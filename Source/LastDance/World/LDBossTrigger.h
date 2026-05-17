// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LDBossTrigger.generated.h"

class UBoxComponent;
class ALDBossCharacter;
class ALDPlayerController;

UENUM()
enum class EBattleState : uint8
{
	Idle,
	Active,
	Cooldown
};

UCLASS()
class LASTDANCE_API ALDBossTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALDBossTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SpawnBoss();
	void EndCooldown();
	UFUNCTION()
	void BossDeath(ALDBaseCharacter* DeadCharacter);

	void TryRegisterWhenReplicated(TWeakObjectPtr<ALDBossCharacter> WeakBoss, TWeakObjectPtr<ALDPlayerController> WeakPC, int32 RetryCount);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ALDBossCharacter> BossClass;

	UPROPERTY()
	TObjectPtr<ALDBossCharacter> ActiveBoss;

	UPROPERTY(EditAnywhere)
	FTransform BossSpawnTransform;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float ResetCooldown = 10.f;

	EBattleState State = EBattleState::Idle;

	FTimerHandle CooldownTimerHandle;
	FTimerHandle PendingRegisterHandle;

};
