// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/LDHazardActor.h"
#include "Combat/LDHazardPool.h"
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

void ALDHazardActor::SetPool(ULDHazardPool* InPool)
{
	OwningPool = InPool;
}

void ALDHazardActor::InitializeFromRow(const FLDHazardTableRow& Row)
{
	CachedRow = Row;
	bRowInitialized = true;

	// 1. 모양에 맞춰 overlapshape 생성 (이름 미지정: 재사용 시 동일 이름 충돌 방지)
	switch (CachedRow.HazardShapeType)
	{
	case ELDHazardShapeType::Sphere:
	{
		USphereComponent* S = NewObject<USphereComponent>(this);
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
		UBoxComponent* B = NewObject<UBoxComponent>(this);
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

void ALDHazardActor::ActivateFromRow(const FLDHazardTableRow& Row)
{
	if (!HasAuthority())
	{
		return;
	}

	// 풀에서 재사용된 액터일 수 있으므로 이전 상태를 정리한 뒤 재구성
	ResetForReuse();
	InitializeFromRow(Row);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// 활성화 위치/회전을 복제용으로 저장 (클라이언트가 OnRep_Activation에서 적용)
	RepLocation = GetActorLocation();
	RepRotation = GetActorRotation();

	++ActivationId;          // 클라이언트 OnRep_Activation 트리거
	OnHazardActivated();     // 서버측 시각 훅

	StartLifecycle();
}

void ALDHazardActor::Deactivate()
{
	// 서버: 풀로 반납하기 전 비활성화 (Destroy 대체)
	ResetForReuse();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	OnHazardDestroyed();
}

void ALDHazardActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALDHazardActor, CachedRow);
	DOREPLIFETIME(ALDHazardActor, RepLocation);
	DOREPLIFETIME(ALDHazardActor, RepRotation);
	DOREPLIFETIME(ALDHazardActor, ActivationId);
}

// Called when the game starts or when spawned
void ALDHazardActor::BeginPlay()
{
	Super::BeginPlay();
	// 수명/시각 활성화는 ActivateFromRow(서버) 및 OnRep_Activation(클라)에서 구동된다.
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
		if (BC->IsDead()) return false;
	}
	return true;
}

void ALDHazardActor::OnHazardActivated()
{
}

void ALDHazardActor::OnHazardDestroyed()
{
}

void ALDHazardActor::StartLifecycle()
{
	if (!HasAuthority()) return;
	if (!bRowInitialized) return;
	if (!IsValid(OverlapShape)) return;

	if (CachedRow.TelegraphDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TelegraphHandle, this,
			&ALDHazardActor::StartApplyingDamage, CachedRow.TelegraphDuration, false);
		return;
	}

	StartApplyingDamage();
}

void ALDHazardActor::ResetForReuse()
{
	if (UWorld* W = GetWorld())
	{
		FTimerManager& TM = W->GetTimerManager();
		TM.ClearTimer(DamageTickHandle);
		TM.ClearTimer(LifetimeHandle);
		TM.ClearTimer(TelegraphHandle);
	}

	if (IsValid(OverlapShape))
	{
		OverlapShape->DestroyComponent();
	}
	OverlapShape = nullptr;
	bRowInitialized = false;
}

void ALDHazardActor::ReturnToPoolOrDestroy()
{
	if (!HasAuthority()) return;

	if (OwningPool.IsValid())
	{
		OwningPool->ReleaseHazard(this);
	}
	else
	{
		Destroy();
	}
}

void ALDHazardActor::StartApplyingDamage()
{
	if (!bRowInitialized) return;

	ApplyDamageTick();

	if (CachedRow.Lifetime <= 0.f)
	{
		ReturnToPoolOrDestroy();
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
	ReturnToPoolOrDestroy();
}

void ALDHazardActor::OnRep_Activation()
{
	if (HasAuthority())
	{
		return;
	}

	// 클라이언트: 복제된 활성화 위치/회전을 먼저 적용
	// (ReplicateMovement=false 이므로 풀 재사용 시 위치가 자동 복제되지 않는다)
	SetActorLocationAndRotation(RepLocation, RepRotation);

	// 매 활성화마다 형상/시각을 재구성하고 다시 표시한다.
	ResetForReuse();
	InitializeFromRow(CachedRow);
	SetActorHiddenInGame(false);
	OnHazardActivated();
}
