// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/LDCombatTypes.h"
#include "LDAttackComboComponent.generated.h"

class ULDComboDataAsset;
class UAnimInstance;
class ACharacter;
class UAnimSequenceBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTDANCE_API ULDAttackComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULDAttackComboComponent();

	// 캐릭터 입력에서 호출
	void HandleCommand(ELDAttackCommand Command);

	bool IsAttacking() const { return bIsAttacking; }
	float GetCurrentDamageMultiplier() const;

	void OnComboWindowBegin(const UAnimSequenceBase* Animation);
	void OnComboWindowEnd(const UAnimSequenceBase* Animation);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	//================== Cache ==================//
	void BuildComboCache();
	int32 FindNextAction(int32 CurrentIndex, ELDAttackCommand Command) const;
	bool IsValidTransition(int32 FromIndex, int32 ToIndex) const;

	//================== Local Flow ==================//
	void StartAction(int32 ActionIndex);
	void TryConsumeBuffer();
	bool PlayActionMontage(int32 ActionIndex, float StartPosition);
	void OnActionMontageEnded(int32 Serial);
	void ResetCombo();

	void SetAttacking(bool bNewAttacking);
	void ApplyMovementMode();

	//================== Helpers ==================//
	ACharacter* GetOwnerCharacter() const;
	UAnimInstance* GetAnimInstance() const;
	bool IsLocallyControlled() const;
	bool IsRolling() const;
	float GetServerTime() const;
	bool IsCurrentActionMontage(const UAnimSequenceBase* Animation) const;

	//================== RPC ==================//
	UFUNCTION(Server, Reliable)
	void ServerRPC_RequestAction(uint8 ActionIndex, float ClientStartTime, int32 RequestSerial);

	UFUNCTION(Server, Reliable)
	void ServerRPC_ActionEnd();

	UFUNCTION(Client, Reliable)
	void ClientRPC_RejectAction(int32 RequestSerial);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_PlayAction(uint8 ActionIndex, float ServerStartTime);

	UFUNCTION()
	void OnRep_IsAttacking();

protected:
	UPROPERTY(EditAnywhere, Category = "Combo")
	TObjectPtr<ULDComboDataAsset> ComboData;

private:
	// 이름 → Actions 인덱스
	TMap<FName, int32> ActionIndexMap;

	// (현재 액션 이름, 커맨드) → 다음 액션 인덱스. 시작 전이는 현재 = NAME_None
	TMap<TPair<FName, ELDAttackCommand>, int32> TransitionMap;

	int32 CurrentActionIndex = INDEX_NONE;
	ELDAttackCommand BufferedCommand = ELDAttackCommand::None;
	bool bComboWindowOpen = false;

	// 액션 시작·리셋마다 증가. 지나간 몽타주의 종료 콜백을 걸러냄
	int32 ActionSerial = 0;

	UPROPERTY(ReplicatedUsing = OnRep_IsAttacking)
	bool bIsAttacking = false;
};
