// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SpawnHazard.h"
#include "Combat/LDHazardActor.h"
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

    ALDHazardActor* Spawned = World->SpawnActorDeferred<ALDHazardActor>(
        HazardClass, SpawnTransform, Owner, InstigatorPawn,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (Spawned)
    {
        Spawned->InitializeFromRow(*Row);
        Spawned->FinishSpawning(SpawnTransform);
    }
}
