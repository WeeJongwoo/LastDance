// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LDBaseCharacter.h"
#include "InputActionValue.h"
#include "Types/LDCombatTypes.h"
#include "LDPlayerCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class ULDAttackComboComponent;

// 입력 액션 하나를 공격 커맨드 하나에 연결
USTRUCT(BlueprintType)
struct FLDAttackInputBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputAction> Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ELDAttackCommand Command = ELDAttackCommand::LMB;
};

/**
 *
 */
UCLASS()
class LASTDANCE_API ALDPlayerCharacter : public ALDBaseCharacter
{
	GENERATED_BODY()
	
public:
	ALDPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	virtual void PossessedBy(AController* NewController) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnAttackCommand(ELDAttackCommand Command);
	void Roll(const FInputActionValue& Value);

	void OnAttackHit(const FHitResult& HitResult) override;

	virtual void SetupCharacterStats() override;

	virtual void HandleDeath() override;

protected:

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ULDAttackComboComponent> AttackComboComponent;

	//================== Input ==================//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RollAction;

	// 입력 액션 → 공격 커맨드. 커맨드를 추가할 때 이 배열에 항목만 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (TitleProperty = "Command"))
	TArray<FLDAttackInputBinding> AttackInputBindings;

	//================== Animation ==================//
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> RollMontage;
};
