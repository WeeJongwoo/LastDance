// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LDCombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"


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
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner && Owner->IsLocallyControlled())
	{
		HitActors.Empty();
		bAttackTraceActive = true;
	}
}

void ULDCombatComponent::EndAttackTrace()
{
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner && Owner->IsLocallyControlled())
	{
		HitActors.Empty();
		bAttackTraceActive = false;
	}
}

void ULDCombatComponent::ExecuteAttackTraceSamples(const TArray<FWeaponTraceSample>& Samples, const FWeaponTraceParams& Params)
{
	if (!bAttackTraceActive || Samples.Num() == 0)
		return;

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
		const FWeaponTraceSample& Sample = Samples[i];

		if (Params.bUseBladeSurface && i > 0)
		{
			PerformBladeSurfaceSweep(World, Samples[i - 1], Sample, QueryParams, Params);
		}
		else
		{
			TArray<FHitResult> HitResults;
			PerformSingleTrace(World, Sample.StartLocation, Sample.EndLocation, QueryParams, Params, HitResults);

#if ENABLE_DRAW_DEBUG
			if (Params.bShowDebugTrace)
			{
				DrawDebugLine(World, Sample.StartLocation, Sample.EndLocation, FColor::Cyan, false, 0.5f, 0, 1.0f);
			}
#endif

			ProcessHits(HitResults, Params.Damage);
		}
	}
}

void ULDCombatComponent::PerformSingleTrace(UWorld* World, const FVector& Start, const FVector& End,
	const FCollisionQueryParams& QueryParams, const FWeaponTraceParams& Params,
	TArray<FHitResult>& OutHits)
{
	if (!World)
		return;

	OutHits.Reset();

	switch (Params.TraceType)
	{
	case EWeaponTraceType::Line:
	{
		World->LineTraceMultiByChannel(
			OutHits,
			Start,
			End,
			ECC_GameTraceChannel3,
			QueryParams
		);
	}
	break;

	case EWeaponTraceType::Sphere:
	{
		FCollisionShape SphereShape = FCollisionShape::MakeSphere(Params.SphereRadius);
		World->SweepMultiByChannel(
			OutHits,
			Start,
			End,
			FQuat::Identity,
			ECC_GameTraceChannel3,
			SphereShape,
			QueryParams
		);
	}
	break;

	case EWeaponTraceType::Box:
	{
		FCollisionShape BoxShape = FCollisionShape::MakeBox(Params.BoxHalfSize);

		FVector Direction = (End - Start).GetSafeNormal();
		FQuat Rotation = FRotationMatrix::MakeFromX(Direction).ToQuat();

		World->SweepMultiByChannel(
			OutHits,
			Start,
			End,
			Rotation,
			ECC_GameTraceChannel3,
			BoxShape,
			QueryParams
		);
	}
	break;
	}
}

void ULDCombatComponent::PerformBladeSurfaceSweep(UWorld* World,
	const FWeaponTraceSample& Prev, const FWeaponTraceSample& Current,
	const FCollisionQueryParams& QueryParams, const FWeaponTraceParams& Params)
{
	if (!World)
		return;

	auto TraceAndProcess = [&](const FVector& Start, const FVector& End, const FColor& DebugColor)
		{
			TArray<FHitResult> HitResults;
			PerformSingleTrace(World, Start, End, QueryParams, Params, HitResults);
			ProcessHits(HitResults, Params.Damage);

#if ENABLE_DRAW_DEBUG
			if (Params.bShowDebugTrace)
			{
				DrawDebugLine(World, Start, End, DebugColor, false, 2.0f, 0, 2.0f);
			}
#endif
		};

	// 1: 현재 프레임 무기 라인
	TraceAndProcess(Current.StartLocation, Current.EndLocation, FColor::Green);

	// 2: 이전 프레임 무기 라인
	TraceAndProcess(Prev.StartLocation, Prev.EndLocation, FColor::Yellow);

	// 3: 대각선 (이전 시작 → 현재 끝)
	TraceAndProcess(Prev.StartLocation, Current.EndLocation, FColor::Red);

	// 4: 대각선 (이전 끝 → 현재 시작)
	TraceAndProcess(Prev.EndLocation, Current.StartLocation, FColor::Red);

	// 5: 시작점 이동 궤적
	TraceAndProcess(Prev.StartLocation, Current.StartLocation, FColor::Red);

	// 6: 끝점 이동 궤적
	TraceAndProcess(Prev.EndLocation, Current.EndLocation, FColor::Red);
}

void ULDCombatComponent::ProcessHits(const TArray<FHitResult>& HitResults, float Damage)
{
	ServerRPC_ProcessHit(HitResults, Damage);
}

void ULDCombatComponent::OnWeaponHit(const FHitResult& Hit, float Damage)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor)
		return;

	AActor* OwnerActor = GetOwner();

	UGameplayStatics::ApplyDamage(
		HitActor,
		Damage,
		OwnerActor ? OwnerActor->GetInstigatorController() : nullptr,
		OwnerActor,
		UDamageType::StaticClass()
	);

#if ENABLE_DRAW_DEBUG
	if (UWorld* World = GetWorld())
	{
		DrawDebugSphere(World, Hit.ImpactPoint, 15.0f, 12, FColor::Red, false, 2.0f);
	}
#endif

	UE_LOG(LogTemp, Log, TEXT("Weapon Hit: %s at %s"), *HitActor->GetName(), *Hit.ImpactPoint.ToString());
}

void ULDCombatComponent::ServerRPC_ProcessHit_Implementation(const TArray<FHitResult>& HitResults, float Damage)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			return;
		}

		ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
		
		bool HitCheck = false;
		if (HitCharacter)
		{
			UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
			if (!AnimInstance || AnimInstance->IsAnyMontagePlaying())
			{
				return;
			}

			FVector OwnerLocation = GetOwner()->GetActorLocation();
			FVector HitActorLocation = HitCharacter->GetActorLocation();
			float HitCharacterCapsuleRadius = HitCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

			float Distance = FVector::Distance(OwnerLocation, HitActorLocation) - HitCharacterCapsuleRadius;

			HitCheck = (AttackRange > Distance);
		}
		else
		{
			HitCheck = (AttackRange > FVector::Distance(GetOwner()->GetActorLocation(), Hit.ImpactPoint));
		}


		if (HitCheck && !HitActors.Contains(HitActor))
		{
			HitActors.Add(HitActor);
			OnWeaponHit(Hit, Damage);
		}
	}
}

