// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LDStatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Log/LDLog.h"


// Sets default values for this component's properties
ULDStatComponent::ULDStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bIsDead = false;
	bStatsInitialized = false;
}


// Called when the game starts
void ULDStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OnRep_CurrentHP();
}

void ULDStatComponent::InitializeStats(const FLDCharacterStats& InStats)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	MaxHP = InStats.MaxHP;
	ATK = InStats.ATK;
	DEF = InStats.DEF;
	CurrentHP = MaxHP;
	bIsDead = false;
	bStatsInitialized = true;

	OnRep_CurrentHP();

	LD_LOG(LDLog, Log, TEXT("Stats Initialized: HP=%.1f, ATK=%.1f, DEF=%.1f"), MaxHP, ATK, DEF);
}

float ULDStatComponent::ApplyDamage(float RawDamage)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bIsDead)
	{
		return 0.0f;
	}

	float ActualDamage = FMath::Max(0.0f, RawDamage - DEF);
	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.0f, MaxHP);

	OnRep_CurrentHP();

	LD_LOG(LDLog, Log, TEXT("ApplyDamage: Raw=%.1f, DEF=%.1f, Actual=%.1f, HP=%.1f/%.1f"),
		RawDamage, DEF, ActualDamage, CurrentHP, MaxHP);

	if (CurrentHP <= 0.0f)
	{
		bIsDead = true;
		OnRep_bIsDead();
	}

	return ActualDamage;
}

void ULDStatComponent::SetAttackRange(float NewRange)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		AttackRange = NewRange;
	}
}

void ULDStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 전체 클라이언트에 복제 (HP바)
	DOREPLIFETIME(ULDStatComponent, CurrentHP);
	DOREPLIFETIME(ULDStatComponent, bIsDead);

	// 본인에게만 복제
	DOREPLIFETIME_CONDITION(ULDStatComponent, MaxHP, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ULDStatComponent, ATK, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ULDStatComponent, DEF, COND_OwnerOnly);
}

void ULDStatComponent::OnRep_CurrentHP()
{
	OnCurrentHPChanged.Broadcast(CurrentHP, MaxHP);
}

void ULDStatComponent::OnRep_bIsDead()
{
	if (bIsDead)
	{
		OnDeath.Broadcast();
	}
}


