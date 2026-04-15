// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/LDCombatInterface.h"
#include "LDBaseCharacter.generated.h"


UCLASS()
class LASTDANCE_API ALDBaseCharacter : public ACharacter, public ILDCombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALDBaseCharacter();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure)
	ULDStatComponent* GetStatComponent() const { return StatComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual ULDCombatComponent* GetCombatComponent() const;

protected:
	virtual void HandleDeath();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_HandleDeath();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ULDCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<ULDStatComponent> StatComponent;
};
