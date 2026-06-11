// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LDHazardPool.generated.h"

class ALDHazardActor;

/**
 * Object Pool 패턴.
 * Hazard 액터를 Spawn/Destroy 반복하지 않고 재사용한다.
 * 서버 권한에서만 운용된다(클라이언트에서는 FreeList가 비어 있음).
 */
UCLASS()
class LASTDANCE_API ULDHazardPool : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 비활성 풀에서 같은 클래스의 액터를 꺼내거나, 없으면 새로 Spawn 한다.
	ALDHazardActor* AcquireHazard(TSubclassOf<ALDHazardActor> HazardClass,
		const FTransform& SpawnTransform, AActor* Owner, APawn* InstigatorPawn);

	// 액터를 비활성화하고 풀로 반납한다(Destroy 대체).
	void ReleaseHazard(ALDHazardActor* Hazard);

private:
	// 재사용 대기 중인(비활성) 액터 목록
	UPROPERTY()
	TArray<TObjectPtr<ALDHazardActor>> FreeList;
};
