// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/LDHazardActor.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/LDBaseCharacter.h"
#include "Log/LDLog.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ALDHazardActor::ALDHazardActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(SceneRoot);
	DecalComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	DecalComp->SetRelativeLocation(FVector(0.f, 0.f, -150.f));
}

void ALDHazardActor::InitializeFromRow(const FLDHazardTableRow& Row)
{
	CachedRow = Row;
	bRowInitialized = true;

	// 1. 모양에 맞춰 overlapshape 생성
	switch (CachedRow.HazardShapeType)
	{
	case ELDHazardShapeType::Sphere:
	{
		USphereComponent* S = NewObject<USphereComponent>(this, TEXT("OverlapSphere"));
		S->InitSphereRadius(CachedRow.SphereRadius);
		OverlapShape = S;

		if (DecalComp)
		{
			const float r = CachedRow.SphereRadius;
			DecalComp->DecalSize = FVector(1000.f, r, r);   // x=깊이, y/z=반지름
		}

		break;
	}
	case ELDHazardShapeType::Box:
	{
		UBoxComponent* B = NewObject<UBoxComponent>(this, TEXT("OverlapBox"));
		B->InitBoxExtent(CachedRow.BoxExtent);
		OverlapShape = B;
		if (DecalComp)
		{
			DecalComp->DecalSize = FVector(CachedRow.BoxExtent.Z * 2.f, CachedRow.BoxExtent.X, CachedRow.BoxExtent.Y);
		}

		break;
	}
	}

	if (!OverlapShape) return;

	OverlapShape->SetupAttachment(SceneRoot);
	OverlapShape->RegisterComponent();
	if (HasAuthority())
	{
		OverlapShape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapShape->SetCollisionObjectType(ECC_GameTraceChannel5);
		OverlapShape->SetCollisionResponseToAllChannels(ECR_Ignore);
		OverlapShape->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	}
	else
	{
		OverlapShape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// 3. 시각 에셋 주입
	if (DecalComp && !CachedRow.GroundDecal.IsNull())
	{
		DecalComp->SetDecalMaterial(CachedRow.GroundDecal);
		DecalComp->SetActive(true, true);
	}
}

void ALDHazardActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALDHazardActor, CachedRow);
}

// Called when the game starts or when spawned
void ALDHazardActor::BeginPlay()
{
	Super::BeginPlay();


	OnHazardActivated();  // 양쪽 머신: 시각 켜기

	if (!HasAuthority()) return;
	if (!bRowInitialized) return;
	if (!IsValid(OverlapShape)) return;  // InitializeFromData가 안 불렸을 경우 방어;

	if (CachedRow.TelegraphDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TelegraphHandle, this,
			&ALDHazardActor::StartApplyingDamage, CachedRow.TelegraphDuration, false);
		return;
	}

	StartApplyingDamage();
}

void ALDHazardActor::EndPlay(const EEndPlayReason::Type Reason)
{
	GetWorldTimerManager().ClearTimer(DamageTickHandle);
	GetWorldTimerManager().ClearTimer(LifetimeHandle);
	GetWorldTimerManager().ClearTimer(TelegraphHandle);
	OnHazardDestroyed();
	Super::EndPlay(Reason);
}

bool ALDHazardActor::ShouldDamageActor(AActor* Target) const
{
	if (!Target || Target == this) return false;
	if (Target == GetInstigator()) return false;

	if (const ALDBaseCharacter* BC = Cast<ALDBaseCharacter>(Target))
	{
		if (BC->IsDead()) return false;  // 함수명은 실제 베이스 보고 맞추기
	}
	return true;
}

void ALDHazardActor::OnHazardActivated()
{
}

void ALDHazardActor::OnHazardDestroyed()
{
}

void ALDHazardActor::StartApplyingDamage()
{
	if (!bRowInitialized) return;

	ApplyDamageTick();

	if (CachedRow.Lifetime <= 0.f)
	{
		Destroy();
		return;
	}

	if (CachedRow.TickInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(DamageTickHandle, this, &ALDHazardActor::ApplyDamageTick, CachedRow.TickInterval, true);
	}

	if (CachedRow.Lifetime > 0.f)
	{
		GetWorldTimerManager().SetTimer(LifetimeHandle, this, &ALDHazardActor::OnLifetimeExpired, CachedRow.Lifetime, false);
	}

}

void ALDHazardActor::ApplyDamageTick()
{

	if (!OverlapShape || !bRowInitialized)
	{
		return;
	}

	OverlapShape->UpdateOverlaps();

	TArray<AActor*> Overlapping;
	OverlapShape->GetOverlappingActors(Overlapping, ALDBaseCharacter::StaticClass());

	for (AActor* T : Overlapping)
	{
		LD_LOG(LDLog, Log, TEXT("Applying Hazard Damage Tick"));
		if (!ShouldDamageActor(T))
		{
			continue;
		}
		UGameplayStatics::ApplyDamage(T, CachedRow.DamagePerTick, GetInstigatorController(), this, UDamageType::StaticClass());
	}
}

void ALDHazardActor::OnLifetimeExpired()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

void ALDHazardActor::OnRep_CachedRow()
{
	if (!OverlapShape)
	{
		InitializeFromRow(CachedRow);
	}
}
