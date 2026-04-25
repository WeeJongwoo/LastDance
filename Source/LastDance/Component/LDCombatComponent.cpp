// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LDCombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Log/LDLog.h"
#include "Net/UnrealNetwork.h"




ULDCombatComponent::ULDCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}


void ULDCombatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	HitActors.Empty();
}

void ULDCombatComponent::BeginAttackTrace()
{
	//LD_LOG(LDLog, Log, TEXT("Begin"));

	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner && Owner->IsLocallyControlled())
	{
		HitActors.Empty();
		bAttackTraceActive = true;
		ServerRPC_AttackTraceStart();
	}
}

void ULDCombatComponent::EndAttackTrace()
{
	//LD_LOG(LDLog, Log, TEXT("Begin"));

	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner && Owner->IsLocallyControlled())
	{
		ServerRPC_AttackTraceEnd();
		HitActors.Empty();
		bAttackTraceActive = false;
	}
}

void ULDCombatComponent::ExecuteAttackTraceSamples(const TArray<FAttackTraceSample>& Samples, const FAttackTraceParams& Params, ETraceChannelType Channel)
{
	if (!bAttackTraceActive || Samples.Num() == 0)
	{
		return;
	}
		
	UWorld* World = GetWorld();
	if (!World)
		return;

	AActor* OwnerActor = GetOwner();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;

	for (int32 i = 0; i < Samples.Num(); ++i)
	{
		const FAttackTraceSample& Sample = Samples[i];

		if (Params.bUseBladeSurface && i > 0)
		{
			PerformBladeSurfaceSweep(World, Samples[i - 1], Sample, QueryParams, Params, Channel);
		}
		else
		{
			TArray<FHitResult> HitResults;
			PerformSingleTrace(World, Sample.StartLocation, Sample.EndLocation, QueryParams, Channel,Params, HitResults);

#if ENABLE_DRAW_DEBUG
			if (Params.bShowDebugTrace)
			{
				DrawDebugLine(World, Sample.StartLocation, Sample.EndLocation, FColor::Cyan, false, 0.5f, 0, 1.0f);
			}
#endif

			ProcessHits(HitResults);
		}
	}
}

void ULDCombatComponent::PerformAttackTrace(const FVector& Start, const FVector& End, ETraceChannelType Channel, const FAttackTraceParams& Params)
{
	TArray<FHitResult> HitResults;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;

	PerformSingleTrace(World, Start, End, QueryParams, Channel, Params, HitResults);

	if (HitResults.Num() == 0)
	{
		return;
	}

	HitActors.Empty();  // 일회성이므로 이전 히트 기록 초기화
	ProcessHits(HitResults);
}

void ULDCombatComponent::PerformSingleTrace(UWorld* World, const FVector& Start, const FVector& End,
	const FCollisionQueryParams& QueryParams, ETraceChannelType TraceChannel, const FAttackTraceParams& Params,
	TArray<FHitResult>& OutHits)
{
	if (!World)
		return;

	OutHits.Reset();

	ECollisionChannel CollisionChannel = GetCollisionChannel(TraceChannel);

	switch (Params.TraceType)
	{
	case EAttackTraceType::Line:
	{
		World->LineTraceMultiByChannel(
			OutHits,
			Start,
			End,
			CollisionChannel,
			QueryParams
		);
	}
	break;

	case EAttackTraceType::Sphere:
	{
		FCollisionShape SphereShape = FCollisionShape::MakeSphere(Params.SphereRadius);
		World->SweepMultiByChannel(
			OutHits,
			Start,
			End,
			FQuat::Identity,
			CollisionChannel,
			SphereShape,
			QueryParams
		);
	}
	break;

	case EAttackTraceType::Box:
	{
		FCollisionShape BoxShape = FCollisionShape::MakeBox(Params.BoxHalfSize);

		FVector Direction = (End - Start).GetSafeNormal();
		FQuat Rotation = FRotationMatrix::MakeFromX(Direction).ToQuat();

		World->SweepMultiByChannel(
			OutHits,
			Start,
			End,
			Rotation,
			CollisionChannel,
			BoxShape,
			QueryParams
		);
	}
	break;
	}
}

void ULDCombatComponent::PerformBladeSurfaceSweep(UWorld* World,
	const FAttackTraceSample& Prev, const FAttackTraceSample& Current,
	const FCollisionQueryParams& QueryParams, const FAttackTraceParams& Params, ETraceChannelType Channel)
{
	if (!World)
		return;

	TArray<FHitResult> HitResults;

	auto TraceAndProcess = [&](const FVector& Start, const FVector& End)
		{
			
			PerformSingleTrace(World, Start, End, QueryParams, Channel, Params, HitResults);
			ProcessHits(HitResults);

#if ENABLE_DRAW_DEBUG
			if (Params.bShowDebugTrace)
			{
				FColor DebugColor = (HitResults.Num() > 0) ? FColor::Green : FColor::Red;
				DrawDebugLine(World, Start, End, DebugColor, false, 2.0f, 0, 2.0f);
			}
#endif
		};

	// 1: 현재 프레임 무기 라인
	TraceAndProcess(Current.StartLocation, Current.EndLocation);

	// 2: 이전 프레임 무기 라인
	TraceAndProcess(Prev.StartLocation, Prev.EndLocation);

	// 3: 대각선 (이전 시작 → 현재 끝)
	TraceAndProcess(Prev.StartLocation, Current.EndLocation);

	// 4: 대각선 (이전 끝 → 현재 시작)
	TraceAndProcess(Prev.EndLocation, Current.StartLocation);

	// 5: 시작점 이동 궤적
	TraceAndProcess(Prev.StartLocation, Current.StartLocation);

	// 6: 끝점 이동 궤적
	TraceAndProcess(Prev.EndLocation, Current.EndLocation);
}

void ULDCombatComponent::ProcessHits(const TArray<FHitResult>& HitResults)
{
	ServerRPC_ProcessHit(HitResults);
}

ECollisionChannel ULDCombatComponent::GetCollisionChannel(ETraceChannelType Channel) const
{
	switch (Channel)
	{
	case ETraceChannelType::Player:
		return ECC_GameTraceChannel1; // 플레이어 공격 채널
	case ETraceChannelType::Enemy:
		return ECC_GameTraceChannel4; // 적 공격 채널
	case ETraceChannelType::Any:
		return ECC_GameTraceChannel3; // 일반 공격 채널
	default:
		return ECollisionChannel::ECC_Pawn;
	}
}

void ULDCombatComponent::ServerRPC_AttackTraceStart_Implementation()
{
	HitActors.Empty();
}

void ULDCombatComponent::ServerRPC_AttackTraceEnd_Implementation()
{
	HitActors.Empty();
}

void ULDCombatComponent::ServerRPC_ProcessHit_Implementation(const TArray<FHitResult>& HitResults)
{
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		if (!HitActors.Contains(HitActor))
		{
			HitActors.Add(HitActor);
			OnAttackHitDelegate.Execute(Hit);
		}
	}
}

