// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LDEnemyCharacter.h"
#include "AI/LDBossPattern.h"
#include "LDBossCharacter.generated.h"

class ALDPlayerController;
class ALDPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCounterStanceFinished, bool, bTriggered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackMontageEnded);

UCLASS()
class LASTDANCE_API ALDBossCharacter : public ALDEnemyCharacter
{
	GENERATED_BODY()
	
	
public:
	ALDBossCharacter(const FObjectInitializer& ObjectInitializer);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	void RegisterController(ALDPlayerController* PC);
	void UnregisterController(ALDPlayerController* PC);

	TArray<ALDPlayerCharacter*> GetAliveTargetPawns() const;

	void EnterCounterStance();
	void PlayAttackMontage();

	FOnAttackMontageEnded OnAttackMontageEnded;
	FOnCounterStanceFinished OnCounterStanceFinished;

	bool IsInCounterStance() const { return bCounterStanceActive; }
	void ExitCounterStance(bool bTriggered);
	bool IsCounterSequenceActive() const { return bCounterStanceActive || bIsCounterAttacking; }

	void SetCounterHitEnabled() { bCounterHitEnabled = true; }

	FString GetBossName() const { return FString(TEXT("Boss")); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleDeath() override;

	//UFUNCTION()
	//void OnPlayerPossessedHandler(ALDPlayerController* PC);

	UFUNCTION()
	void OnPlayerLeftHandler(ALDPlayerController* PC);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage(FName SectionName);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayCounterMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_JumpToCounterAttackSection();

	void TriggerCounter();

	UFUNCTION()
	void HandleCounterMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void AttackMontageEndedHandler(UAnimMontage* Montage, bool bInterrupted);

	void RegistedCharacterDead(ALDBaseCharacter* DeadCharacter);


protected:

	UPROPERTY()
	TArray<TWeakObjectPtr<ALDPlayerController>> KnownControllers;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> CounterMontage;

	FName CounterWaitSection = "Wait";

	FName CounterAttackSection = "Attack";

	UPROPERTY(EditAnywhere, Category = "Counter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CounterDamageMultiplier = 0.3f;

	bool bCounterStanceActive = false;
	bool bIsCounterAttacking = false;
	bool bCounterHitEnabled = false;
};
