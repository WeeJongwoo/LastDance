// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/LDCharacterStats.h"
#include "LDStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentHPChanged, float, NewHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTDANCE_API ULDStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULDStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	void InitializeStats(const FLDCharacterStats& InStats);

	float ApplyDamage(float RawDamage);

	UFUNCTION(BlueprintPure)
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure)
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintPure)
	float GetATK() const { return ATK; }

	UFUNCTION(BlueprintPure)
	float GetDEF() const { return DEF; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable)
	FOnCurrentHPChanged OnCurrentHPChanged;

	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_bIsDead();

protected:
	// 전체 클라이언트에 리플리케이트 (HP바 표시용)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float CurrentHP = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_bIsDead)
	uint8 bIsDead : 1;

	// 본인에게만 리플리케이트 (COND_OwnerOnly)
	UPROPERTY(Replicated)
	float MaxHP = 100.0f;

	UPROPERTY(Replicated)
	float ATK = 10.0f;

	UPROPERTY(Replicated)
	float DEF = 5.0f;

	uint8 bStatsInitialized : 1;
	
};
