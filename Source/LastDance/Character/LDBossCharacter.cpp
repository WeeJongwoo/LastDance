// Fill out your copyright notice in the Description page of Project Settings.


#include "LDBossCharacter.h"
#include "LDPlayerCharacter.h"
#include "Player/LDPlayerController.h"
#include "Player/LDPlayerState.h"
#include "GameMode/LDGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Log/LDLog.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

ALDBossCharacter::ALDBossCharacter()
{
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

float ALDBossCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bCounterHitEnabled && bCounterStanceActive)
	{
		const float Reduced = DamageAmount * CounterDamageMultiplier;
		const float Applied = Super::TakeDamage(Reduced, DamageEvent, EventInstigator, DamageCauser);
		TriggerCounter();
		return Applied;
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ALDBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState<AGameStateBase>())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (!PS)
				{
					continue;
				}

				if (ALDPlayerController* PC = Cast<ALDPlayerController>(PS->GetPlayerController()))
				{
					RegisterController(PC);
				}
			}
		}
	}

	if (ALDGameMode* GameMode = GetWorld()->GetAuthGameMode<ALDGameMode>())
	{
		GameMode->OnPlayerPossessed.AddDynamic(this, &ALDBossCharacter::OnPlayerPossessedHandler);
		GameMode->OnPlayerLeft.AddDynamic(this, &ALDBossCharacter::OnPlayerLeftHandler);
	}
}

void ALDBossCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (ALDGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALDGameMode>() : nullptr)
		{
			GameMode->OnPlayerPossessed.RemoveDynamic(this, &ALDBossCharacter::OnPlayerPossessedHandler);
			GameMode->OnPlayerLeft.RemoveDynamic(this, &ALDBossCharacter::OnPlayerLeftHandler);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ALDBossCharacter::HandleDeath()
{
	if (HasAuthority())
	{
		for (TWeakObjectPtr<ALDPlayerController>& Weak : KnownControllers)
		{
			if (ALDPlayerController* PlayerController = Weak.Get())
			{
				PlayerController->ClientRPC_NotifyBossDefeated(this);
			}
		}
		KnownControllers.Reset();

	}

	Super::HandleDeath();
}

void ALDBossCharacter::RegisterController(ALDPlayerController* PC)
{
	if (!HasAuthority() || !PC)
	{
		return;
	}

	// 중복 검사
	for (const TWeakObjectPtr<ALDPlayerController>& Weak : KnownControllers)
	{
		if (Weak.Get() == PC)
		{
			return;
		}
	}

	KnownControllers.Add(PC);

	LD_LOG(LDLog, Log, TEXT("Boss registered PC: %s (total=%d)"), *GetNameSafe(PC), KnownControllers.Num());

	PC->ClientRPC_NotifyRecognizedByBoss(this);
}

void ALDBossCharacter::UnregisterController(ALDPlayerController* PC)
{
	if (!HasAuthority() || !PC)
	{
		return;
	}

	const int32 Removed = KnownControllers.RemoveAll([PC](const TWeakObjectPtr<ALDPlayerController>& W)
		{
			return !W.IsValid() || W.Get() == PC;
		});

	if (Removed > 0)
	{
		LD_LOG(LDLog, Log, TEXT("Boss unregistered PC: %s (total=%d)"), *GetNameSafe(PC), KnownControllers.Num());
	}
}

TArray<ALDPlayerCharacter*> ALDBossCharacter::GetAliveTargetPawns() const
{
	TArray<ALDPlayerCharacter*> Out;
	Out.Reserve(KnownControllers.Num());

	for (const TWeakObjectPtr<ALDPlayerController>& Weak : KnownControllers)
	{
		ALDPlayerController* PC = Weak.Get();
		if (!PC) continue;
		if (ALDPlayerCharacter* P = PC->GetPawn<ALDPlayerCharacter>())
		{
			if (!P->IsDead())
			{
				Out.Add(P);
			}
		}
	}
	return Out;
}

void ALDBossCharacter::EnterCounterStance()
{
	if (bCounterStanceActive || bIsCounterAttacking || !CounterMontage) return;

	bCounterStanceActive = true;

	// 모든 머신(서버 + 클라)에서 몽타주 재생
	Multicast_PlayCounterMontage();

	// 서버에서만 종료 콜백 바인딩 (BTTask 종료 신호용)
	if (HasAuthority())
	{
		if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			FOnMontageBlendingOutStarted EndDel;
			EndDel.BindUObject(this, &ALDBossCharacter::HandleCounterMontageEnded);
			AnimInst->Montage_SetBlendingOutDelegate(EndDel, CounterMontage);
		}
	}
}

void ALDBossCharacter::PlayAttackMontage()
{
	int32 SectionIndex = FMath::RandRange(1, 4);
	FName AttackSection = FName(*FString::Printf(TEXT("Attack%d"), SectionIndex));

	Multicast_PlayAttackMontage(AttackSection);

	if (HasAuthority())
	{
		if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			FOnMontageBlendingOutStarted EndDel;
			EndDel.BindUObject(this, &ALDBossCharacter::AttackMontageEndedHandler);
			AnimInst->Montage_SetBlendingOutDelegate(EndDel, AttackMontage);
		}
	}
}

void ALDBossCharacter::OnPlayerPossessedHandler(ALDPlayerController* PC)
{
	RegisterController(PC);
}

void ALDBossCharacter::OnPlayerLeftHandler(ALDPlayerController* PC)
{
	UnregisterController(PC);
}

void ALDBossCharacter::Multicast_PlayAttackMontage_Implementation(FName SectionName)
{
	if (!AttackMontage)
	{
		return;
	}

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst)
	{
		return;
	}

	AnimInst->Montage_Stop(0.f, AttackMontage);
	AnimInst->Montage_Play(AttackMontage);
	AnimInst->Montage_JumpToSection(SectionName, AttackMontage);
}

void ALDBossCharacter::Multicast_PlayCounterMontage_Implementation()
{
	if (!CounterMontage)
	{
		return;
	}

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst)
	{
		return;
	}

	AnimInst->Montage_Stop(0.f, CounterMontage);
	AnimInst->Montage_Play(CounterMontage);
	AnimInst->Montage_JumpToSection(CounterWaitSection, CounterMontage);
}

void ALDBossCharacter::Multicast_JumpToCounterAttackSection_Implementation()
{
	if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInst->Montage_JumpToSection(CounterAttackSection, CounterMontage);
	}
}

void ALDBossCharacter::TriggerCounter()
{
	if (!bCounterStanceActive || !bCounterHitEnabled)
	{
		return;
	}

	bCounterStanceActive = false;
	bCounterHitEnabled = false;
	bIsCounterAttacking = true;

	Multicast_JumpToCounterAttackSection();

}

void ALDBossCharacter::ExitCounterStance(bool bTriggered)
{
	bIsCounterAttacking = false;
	bCounterStanceActive = false;
	bCounterHitEnabled = false;
	OnCounterStanceFinished.Broadcast(bTriggered);
}

void ALDBossCharacter::HandleCounterMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted) return;
	if (Montage != CounterMontage) return;


	if (bIsCounterAttacking)
	{
		ExitCounterStance(true);
	}
	if (bCounterStanceActive)
	{
		ExitCounterStance(false);
	}
}

void ALDBossCharacter::AttackMontageEndedHandler(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted) return;
	if (Montage != AttackMontage) return;

	OnAttackMontageEnded.Broadcast();
}
