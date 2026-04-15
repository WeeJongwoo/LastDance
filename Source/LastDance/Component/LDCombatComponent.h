// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/LDCombatTypes.h"
#include "LDCombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTDANCE_API ULDCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULDCombatComponent();

	// 공격 트레이스 시작/종료
	void BeginAttackTrace();
	void EndAttackTrace();

	// AnimNotify에서 호출: 샘플 배열로 트레이스 실행
	void ExecuteAttackTraceSamples(const TArray<FWeaponTraceSample>& Samples, const FWeaponTraceParams& Params);

	virtual void InitializeComponent() override;

private:
	// 단일 충돌 검사 수행 (타입에 따라 Line/Sphere/Box)
	void PerformSingleTrace(UWorld* World, const FVector& Start, const FVector& End,
		const FCollisionQueryParams& QueryParams, const FWeaponTraceParams& Params,
		TArray<FHitResult>& OutHits);

	// 검의 면적을 고려한 Sweep (대각선 포함)
	void PerformBladeSurfaceSweep(UWorld* World,
		const FWeaponTraceSample& Prev, const FWeaponTraceSample& Current,
		const FCollisionQueryParams& QueryParams, const FWeaponTraceParams& Params);

	// 히트 처리
	void ProcessHits(const TArray<FHitResult>& HitResults, float Damage);
	void OnWeaponHit(const FHitResult& Hit, float Damage);

	UFUNCTION(Server, Reliable)
	void ServerRPC_ProcessHit(const TArray<FHitResult>& HitResults, float Damage);

	UFUNCTION(Server, Reliable)
	void ServerRPC_AttackTraceStart();

	UFUNCTION(Server, Reliable)
	void ServerRPC_AttackTraceEnd();

	// 이미 히트한 액터 (중복 방지)
	TSet<AActor*> HitActors;

	bool bAttackTraceActive = false;
		
	float AttackRange = 300.0f; // 공격 범위 (예시값)
};
