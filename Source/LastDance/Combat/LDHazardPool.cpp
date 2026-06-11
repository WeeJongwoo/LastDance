// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/LDHazardPool.h"
#include "Combat/LDHazardActor.h"
#include "Engine/World.h"
#include "Log/LDLog.h"

ALDHazardActor* ULDHazardPool::AcquireHazard(TSubclassOf<ALDHazardActor> HazardClass,
	const FTransform& SpawnTransform, AActor* Owner, APawn* InstigatorPawn)
{
	UWorld* World = GetWorld();
	if (!World || !*HazardClass)
	{
		return nullptr;
	}

	ALDHazardActor* Hazard = nullptr;

	// 1. 같은 클래스의 비활성 액터를 풀에서 탐색
	for (int32 i = FreeList.Num() - 1; i >= 0; --i)
	{
		ALDHazardActor* Candidate = FreeList[i];
		if (IsValid(Candidate) && Candidate->GetClass() == HazardClass)
		{
			Hazard = Candidate;
			FreeList.RemoveAtSwap(i);
			break;
		}
	}

	if (Hazard)
	{
		// 재사용: 위치/소유자만 갱신 (활성화는 ActivateFromRow가 담당)
		Hazard->SetActorTransform(SpawnTransform);
		Hazard->SetOwner(Owner);
		Hazard->SetInstigator(InstigatorPawn);
		LD_LOG(LDLog, Log, TEXT("[HazardPool] Reuse pooled hazard (remaining free=%d)"), FreeList.Num());
	}
	else
	{
		// 풀이 비었으면 새로 생성하고 풀 소유권을 등록
		FActorSpawnParameters Params;
		Params.Owner = Owner;
		Params.Instigator = InstigatorPawn;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		Hazard = World->SpawnActor<ALDHazardActor>(HazardClass, SpawnTransform, Params);
		if (Hazard)
		{
			Hazard->SetPool(this);
			LD_LOG(LDLog, Log, TEXT("[HazardPool] Spawn new hazard"));
		}
	}

	return Hazard;
}

void ULDHazardPool::ReleaseHazard(ALDHazardActor* Hazard)
{
	if (!IsValid(Hazard))
	{
		return;
	}
	if (FreeList.Contains(Hazard))
	{
		return; // 이미 반납됨 (중복 반납 방어)
	}

	Hazard->Deactivate();
	FreeList.Add(Hazard);
	LD_LOG(LDLog, Log, TEXT("[HazardPool] Release hazard (free=%d)"), FreeList.Num());
}
