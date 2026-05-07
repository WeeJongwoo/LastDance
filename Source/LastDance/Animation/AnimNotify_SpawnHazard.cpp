// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SpawnHazard.h"
#include "Combat/LDHazardActor.h"
#include "Data/LDHazardDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

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

    if (!HazardClass || !HazardData)
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

    ALDHazardActor* Spawned = World->SpawnActorDeferred<ALDHazardActor>(HazardClass, SpawnTransform, 
        Owner, InstigatorPawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (!Spawned) return;

    Spawned->InitializeFromData(HazardData);
    Spawned->FinishSpawning(SpawnTransform);
}
