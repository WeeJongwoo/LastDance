// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SpawnHazard.h"
#include "Combat/LDHazardActor.h"
#include "Combat/LDHazardPool.h"
#include "Data/LDHazardDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Data/LDHazardTableRow.h"

void UAnimNotify_SpawnHazard::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner)
    {
        return;
    }

    if (!Owner->HasAuthority())
    {
        return;
    }

    if (!HazardClass || !HazardTable)
    {
        return;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        return;
    }

    FVector SpawnLocation = Owner->GetActorLocation();
    FRotator SpawnRotation = Owner->GetActorRotation();

    if (!SpawnSocketName.IsNone() && MeshComp->DoesSocketExist(SpawnSocketName))
    {
        SpawnLocation = MeshComp->GetSocketLocation(SpawnSocketName);
    }

    SpawnLocation += Owner->GetActorRotation().RotateVector(SpawnOffset);

    const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

    APawn* InstigatorPawn = Cast<APawn>(Owner);

    if (!HazardTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimNotify_SpawnHazard: HazardTable not assigned"));
        return;
    }

    const FLDHazardTableRow* Row = HazardTable->FindRow<FLDHazardTableRow>(
        HazardRowName, TEXT("AnimNotify_SpawnHazard"));
    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimNotify_SpawnHazard: Row '%s' not found"),
            *HazardRowName.ToString());
        return;
    }

    // Object Pool: 직접 Spawn/Destroy 대신 풀에서 빌려쓴다.
    ULDHazardPool* Pool = World->GetSubsystem<ULDHazardPool>();
    if (!Pool)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimNotify_SpawnHazard: ULDHazardPool subsystem not found"));
        return;
    }

    ALDHazardActor* Hazard = Pool->AcquireHazard(HazardClass, SpawnTransform, Owner, InstigatorPawn);
    if (Hazard)
    {
        Hazard->ActivateFromRow(*Row);
    }
}
