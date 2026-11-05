// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LDAttackComboComponent.h"
#include "Data/LDComboDataAsset.h"
#include "Character/LDCharacterMovementComponent.h"
#include "Character/LDBaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "Log/LDLog.h"

ULDAttackComboComponent::ULDAttackComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULDAttackComboComponent::BeginPlay()
{
	Super::BeginPlay();

	BuildComboCache();
}

void ULDAttackComboComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 오너는 로컬에서 직접 관리하므로 복제에서 제외
	DOREPLIFETIME_CONDITION(ULDAttackComboComponent, bIsAttacking, COND_SkipOwner);
}

//================== Cache ==================//

void ULDAttackComboComponent::BuildComboCache()
{
	ActionIndexMap.Reset();
	TransitionMap.Reset();

	if (!ComboData)
	{
		LD_LOG(LDLog, Warning, TEXT("ComboData가 지정되지 않았습니다."));
		return;
	}

	const TArray<FLDComboAction>& Actions = ComboData->Actions;
	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		const FName Name = Actions[i].ActionName;

		// None은 시작 전이(From = NAME_None)와 겹치고, 중복은 인덱스를 덮어쓰므로 제외 (먼저 나온 것만 사용)
		if (Name.IsNone() || ActionIndexMap.Contains(Name))
		{
			LD_LOG(LDLog, Warning, TEXT("Actions[%d]: 이름이 비었거나 중복('%s')이라 무시합니다."), i, *Name.ToString());
			continue;
		}

		ActionIndexMap.Add(Name, i);
	}

	auto AddTransitions = [this](FName From, const TArray<FLDComboNext>& Nexts)
		{
			for (const FLDComboNext& Next : Nexts)
			{
				const int32* ToIndex = ActionIndexMap.Find(Next.ToAction);
				if (!ToIndex)
				{
					LD_LOG(LDLog, Warning, TEXT("'%s' → '%s': 존재하지 않는 액션이라 무시합니다."), *From.ToString(), *Next.ToAction.ToString());
					continue;
				}

				TransitionMap.Add(MakeTuple(From, Next.Command), *ToIndex);
			}
		};

	AddTransitions(NAME_None, ComboData->StartActions);
	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		// 위에서 제외된 액션의 전이는 등록하지 않음
		const int32* Index = ActionIndexMap.Find(Actions[i].ActionName);
		if (Index && *Index == i)
		{
			AddTransitions(Actions[i].ActionName, Actions[i].NextActions);
		}
	}
}

int32 ULDAttackComboComponent::FindNextAction(int32 CurrentIndex, ELDAttackCommand Command) const
{
	if (!ComboData)
	{
		return INDEX_NONE;
	}

	const FName From = ComboData->Actions.IsValidIndex(CurrentIndex) ? ComboData->Actions[CurrentIndex].ActionName : NAME_None;
	const int32* Next = TransitionMap.Find(MakeTuple(From, Command));

	return Next ? *Next : INDEX_NONE;
}

bool ULDAttackComboComponent::IsValidTransition(int32 FromIndex, int32 ToIndex) const
{
	// From에서 어떤 커맨드로든 To에 갈 수 있으면 유효 (0 = None은 제외)
	for (uint8 i = 1; i < static_cast<uint8>(ELDAttackCommand::MAX); ++i)
	{
		if (FindNextAction(FromIndex, static_cast<ELDAttackCommand>(i)) == ToIndex)
		{
			return true;
		}
	}
	return false;
}

//================== Local Flow ==================//

void ULDAttackComboComponent::HandleCommand(ELDAttackCommand Command)
{
	if (!IsLocallyControlled() || IsRolling())
	{
		return;
	}

	if (CurrentActionIndex == INDEX_NONE)
	{
		const int32 Next = FindNextAction(INDEX_NONE, Command);
		if (Next != INDEX_NONE)
		{
			StartAction(Next);
		}
		return;
	}

	// 공격 중: 마지막 입력을 버퍼에 저장
	BufferedCommand = Command;

	if (bComboWindowOpen)
	{
		TryConsumeBuffer();
	}
}

void ULDAttackComboComponent::OnComboWindowBegin()
{
	if (!IsLocallyControlled() || CurrentActionIndex == INDEX_NONE)
	{
		return;
	}

	bComboWindowOpen = true;
	TryConsumeBuffer();
}

void ULDAttackComboComponent::OnComboWindowEnd()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	bComboWindowOpen = false;
	BufferedCommand = ELDAttackCommand::None;
}

void ULDAttackComboComponent::TryConsumeBuffer()
{
	if (BufferedCommand == ELDAttackCommand::None)
	{
		return;
	}

	const int32 Next = FindNextAction(CurrentActionIndex, BufferedCommand);
	BufferedCommand = ELDAttackCommand::None;

	if (Next != INDEX_NONE)
	{
		StartAction(Next);
	}
}

void ULDAttackComboComponent::StartAction(int32 ActionIndex)
{
	// 재생에 실패하면 상태를 바꾸지 않음 → 진행 중이던 액션(있다면)과 그 종료 콜백이 그대로 유효
	if (!PlayActionMontage(ActionIndex, 0.0f))
	{
		LD_LOG(LDLog, Warning, TEXT("StartAction 실패: Index=%d"), ActionIndex);
		return;
	}

	// 이전 몽타주의 종료 콜백은 큐에 쌓였다가 나중에 호출되므로, 재생 후에 Serial을 올려도 걸러짐
	CurrentActionIndex = ActionIndex;
	BufferedCommand = ELDAttackCommand::None;
	bComboWindowOpen = false;
	const int32 Serial = ++ActionSerial;

	if (UAnimInstance* AnimInstance = GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindWeakLambda(this, [this, Serial](UAnimMontage* Montage, bool bInterrupted)
			{
				OnActionMontageEnded(Serial);
			});

		// Montage_Play 직후에 호출해야 새로 재생된 인스턴스에 바인딩됨
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboData->Actions[ActionIndex].Montage);
	}

	SetAttacking(true);

	LD_LOG(LDLog, Log, TEXT("StartAction: %s"), *ComboData->Actions[ActionIndex].ActionName.ToString());
	ServerRPC_RequestAction(static_cast<uint8>(ActionIndex), GetServerTime(), Serial);
}

bool ULDAttackComboComponent::PlayActionMontage(int32 ActionIndex, float StartPosition)
{
	if (!ComboData || !ComboData->Actions.IsValidIndex(ActionIndex))
	{
		return false;
	}

	UAnimMontage* Montage = ComboData->Actions[ActionIndex].Montage;
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!Montage || !AnimInstance)
	{
		return false;
	}

	// 지연이 몽타주 길이보다 크면 이미 끝난 공격
	if (StartPosition >= Montage->GetPlayLength())
	{
		return false;
	}

	const float PlayLength = AnimInstance->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::MontageLength, FMath::Max(0.0f, StartPosition));
	return PlayLength > 0.0f;
}

void ULDAttackComboComponent::OnActionMontageEnded(int32 Serial)
{
	// 이미 다음 액션으로 넘어갔거나 리셋됨 → 지나간 몽타주의 콜백이므로 무시
	if (Serial != ActionSerial)
	{
		return;
	}

	ResetCombo();
	ServerRPC_ActionEnd();
}

void ULDAttackComboComponent::ResetCombo()
{
	CurrentActionIndex = INDEX_NONE;
	BufferedCommand = ELDAttackCommand::None;
	bComboWindowOpen = false;
	++ActionSerial;

	SetAttacking(false);
}

void ULDAttackComboComponent::SetAttacking(bool bNewAttacking)
{
	bIsAttacking = bNewAttacking;
	ApplyMovementMode();
}

void ULDAttackComboComponent::ApplyMovementMode()
{
	ACharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	// 사망 후에는 DisableMovement가 설정한 MOVE_None을 유지 (몽타주 종료 콜백이 Walking으로 되돌리지 않도록)
	const ALDBaseCharacter* LDCharacter = Cast<ALDBaseCharacter>(Character);
	if (LDCharacter && LDCharacter->IsDeathHandled())
	{
		return;
	}

	if (bIsAttacking)
	{
		MoveComp->SetMovementMode(MOVE_None);
	}
	else if (MoveComp->MovementMode == MOVE_None)
	{
		// 공격으로 멈춘 경우에만 복구 (구르기·낙하 상태는 건드리지 않음)
		MoveComp->SetMovementMode(MOVE_Walking);
	}
}

void ULDAttackComboComponent::OnRep_IsAttacking()
{
	ApplyMovementMode();
}

float ULDAttackComboComponent::GetCurrentDamageMultiplier() const
{
	if (ComboData && ComboData->Actions.IsValidIndex(CurrentActionIndex))
	{
		return ComboData->Actions[CurrentActionIndex].DamageMultiplier;
	}
	return 1.0f;
}

//================== Helpers ==================//

ACharacter* ULDAttackComboComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

UAnimInstance* ULDAttackComboComponent::GetAnimInstance() const
{
	const ACharacter* Character = GetOwnerCharacter();
	return (Character && Character->GetMesh()) ? Character->GetMesh()->GetAnimInstance() : nullptr;
}

bool ULDAttackComboComponent::IsLocallyControlled() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn && Pawn->IsLocallyControlled();
}

bool ULDAttackComboComponent::IsRolling() const
{
	const ACharacter* Character = GetOwnerCharacter();
	const ULDCharacterMovementComponent* MoveComp = Character ? Cast<ULDCharacterMovementComponent>(Character->GetCharacterMovement()) : nullptr;
	return MoveComp && MoveComp->IsRolling();
}

float ULDAttackComboComponent::GetServerTime() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

//================== RPC ==================//

void ULDAttackComboComponent::ServerRPC_RequestAction_Implementation(uint8 ActionIndex, float ClientStartTime, int32 RequestSerial)
{
	// 리슨 서버 호스트는 StartAction에서 이미 같은 객체의 상태를 갱신했으므로 검증 생략
	if (!IsLocallyControlled())
	{
		const bool bValidIndex = ComboData && ComboData->Actions.IsValidIndex(ActionIndex);
		if (!bValidIndex || IsRolling() || !IsValidTransition(CurrentActionIndex, ActionIndex))
		{
			LD_LOG(LDLog, Warning, TEXT("Reject: Current=%d, Request=%d"), CurrentActionIndex, ActionIndex);
			ResetCombo();
			ClientRPC_RejectAction(RequestSerial);
			return;
		}

		CurrentActionIndex = ActionIndex;
		SetAttacking(true);
	}

	MulticastRPC_PlayAction(ActionIndex, ClientStartTime);
}

void ULDAttackComboComponent::ServerRPC_ActionEnd_Implementation()
{
	if (!IsLocallyControlled())
	{
		ResetCombo();
	}
}

void ULDAttackComboComponent::ClientRPC_RejectAction_Implementation(int32 RequestSerial)
{
	// 거절된 요청 이후 이미 리셋됐거나 다른 액션을 시작함 → 지나간 거절이므로 무시
	if (RequestSerial != ActionSerial)
	{
		return;
	}

	// ResetCombo가 Serial을 올리므로, 아래 Montage_Stop이 부르는 종료 콜백은 무시됨
	ResetCombo();

	if (UAnimInstance* AnimInstance = GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.2f);
	}
}

void ULDAttackComboComponent::MulticastRPC_PlayAction_Implementation(uint8 ActionIndex, float ServerStartTime)
{
	if (IsLocallyControlled())
	{
		return;
	}

	const float TimeDiff = GetServerTime() - ServerStartTime;
	PlayActionMontage(ActionIndex, TimeDiff);
}
