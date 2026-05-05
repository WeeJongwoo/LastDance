// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/LDHarzardActor.h"
#include "Data/LDHazardDataAsset.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/LDBaseCharacter.h"
#include "Log/LDLog.h"


// Sets default values
ALDHarzardActor::ALDHarzardActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(SceneRoot);
	DecalComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

}

void ALDHarzardActor::InitializeFromData(const ULDHazardDataAsset* InData)
{
	if (!InData) return;
	HazardData = InData;

	// 1. 모양에 맞춰 OverlapShape 생성
	switch (InData->HazardShapeType)
	{
	case ELDHazardShapeType::Sphere:
	{
		USphereComponent* S = NewObject<USphereComponent>(this, TEXT("OverlapSphere"));
		S->InitSphereRadius(InData->SphereRadius);
		OverlapShape = S;

		if (DecalComp)
		{
			const float R = InData->SphereRadius;
			DecalComp->DecalSize = FVector(64.f, R, R);   // X=깊이, Y/Z=반지름
		}

		break;
	}
	case ELDHazardShapeType::Box:
	{
		UBoxComponent* B = NewObject<UBoxComponent>(this, TEXT("OverlapBox"));
		B->InitBoxExtent(InData->BoxExtent);
		OverlapShape = B;

		if (DecalComp)
		{
			DecalComp->DecalSize = FVector(InData->BoxExtent.Z * 2.f,InData->BoxExtent.X,InData->BoxExtent.Y);
		}

		break;
	}
	}

	if (!OverlapShape) return;

	OverlapShape->SetupAttachment(SceneRoot);
	OverlapShape->RegisterComponent();

	// 2. 콜리전 설정 (서버만 의미 있음. 클라는 어차피 데미지 안 함)
	OverlapShape->SetCollisionEnabled(HasAuthority() ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	OverlapShape->SetCollisionObjectType(ECC_GameTraceChannel5);
	OverlapShape->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapShape->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

	// 3. 시각 에셋 주입
	if (DecalComp && (InData->GroundDecal.IsValid() || !InData->GroundDecal.IsNull()))
	{
		UMaterialInterface* Mat = InData->GroundDecal.LoadSynchronous();
		if (Mat) DecalComp->SetDecalMaterial(Mat);
	}
}

// Called when the game starts or when spawned
void ALDHarzardActor::BeginPlay()
{
	Super::BeginPlay();

	if (HazardData && !OverlapShape)
	{
		InitializeFromData(HazardData);
	}

	OnHazardActivated();  // 양쪽 머신: 시각 켜기

	if (!HasAuthority()) return;
	if (!IsValid(OverlapShape)) return;  // InitializeFromData가 안 불렸을 경우 방어;

	if (HazardData && HazardData->TelegraphDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TelegraphHandle, this, &ALDHarzardActor::StartApplyingDamage, HazardData->TelegraphDuration, false);
		return;
	}

	StartApplyingDamage();
}

void ALDHarzardActor::EndPlay(const EEndPlayReason::Type Reason)
{
	GetWorldTimerManager().ClearTimer(DamageTickHandle);
	GetWorldTimerManager().ClearTimer(LifetimeHandle);
	GetWorldTimerManager().ClearTimer(TelegraphHandle);
	OnHazardDestroyed();
	Super::EndPlay(Reason);
}

bool ALDHarzardActor::ShouldDamageActor(AActor* Target) const
{
	if (!Target || Target == this) return false;
	if (Target == GetInstigator()) return false;

	if (const ALDBaseCharacter* BC = Cast<ALDBaseCharacter>(Target))
	{
		if (BC->IsDead()) return false;  // 함수명은 실제 베이스 보고 맞추기
	}
	return true;
}

void ALDHarzardActor::OnHazardActivated()
{
}

void ALDHarzardActor::OnHazardDestroyed()
{
}

void ALDHarzardActor::StartApplyingDamage()
{
	ApplyDamageTick();

	if (HazardData && HazardData->Lifetime <= 0.f)
	{
		Destroy();
		return;
	}

	// 데미지 적용
	if (HazardData && HazardData->TickInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(DamageTickHandle, this,&ALDHarzardActor::ApplyDamageTick, HazardData->TickInterval, true);  // 첫 틱 즉시
	}

	// 라이프타임
	if (HazardData && HazardData->Lifetime > 0.f)
	{
		GetWorldTimerManager().SetTimer(LifetimeHandle, this, &ALDHarzardActor::OnLifetimeExpired, HazardData->Lifetime, false);
	}
	
}

void ALDHarzardActor::ApplyDamageTick()
{
	
	if (!OverlapShape || !HazardData)
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
		UGameplayStatics::ApplyDamage(T, HazardData->DamagePerTick, GetInstigatorController(), this, UDamageType::StaticClass());
	}
}

void ALDHarzardActor::OnLifetimeExpired()
{
	if (HasAuthority())
	{
		Destroy();
	}
}
