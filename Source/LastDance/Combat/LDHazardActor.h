// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/LDHazardTableRow.h"
#include "LDHazardActor.generated.h"

class UShapeComponent;
class UDecalComponent;
class ULDHazardDataAsset;
class ULDHazardPool;

UCLASS()
class LASTDANCE_API ALDHazardActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALDHazardActor();

    // 서버에서 호출: 데이터 설정 + 동적 형상/시각 구성
    void InitializeFromRow(const FLDHazardTableRow& Row);

    // 서버에서 호출(Object Pool): 풀에서 꺼낸 액터를 데이터로 재초기화하고 수명을 시작한다.
    void ActivateFromRow(const FLDHazardTableRow& Row);

    // 서버에서 호출(Object Pool): 비활성화 후 풀로 반납하기 위한 정리.
    void Deactivate();

    // 풀이 소유 액터에 자신을 등록한다.
    void SetPool(ULDHazardPool* InPool);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    // 서브클래스용 가상 훅
    virtual bool ShouldDamageActor(AActor* Target) const;
    virtual void OnHazardActivated();
    virtual void OnHazardDestroyed();
	void StartApplyingDamage();

private:
    void StartLifecycle();          // 서버: 텔레그래프/데미지 타이머 시작
    void ResetForReuse();           // 타이머/동적 컴포넌트/플래그 초기화(재사용 준비)
    void ReturnToPoolOrDestroy();   // 수명 종료 시 풀 반납 또는 파괴

    void ApplyDamageTick();
    void OnLifetimeExpired();

    // ActivationId 가 복제될 때(매 활성화) 클라이언트에서 형상/시각을 재구성한다.
    UFUNCTION()
    void OnRep_Activation();
protected:

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY()
    TObjectPtr<UShapeComponent> OverlapShape;  // 동적 생성

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UDecalComponent> DecalComp;

    UPROPERTY(Replicated)
    FLDHazardTableRow CachedRow;

    // 풀 재사용 시 SetActorTransform 으로만 이동하므로(ReplicateMovement=false)
    // 활성화 위치/회전을 직접 복제해 클라이언트에서 맞춘다.
    UPROPERTY(Replicated)
    FVector RepLocation = FVector::ZeroVector;

    UPROPERTY(Replicated)
    FRotator RepRotation = FRotator::ZeroRotator;

    // 매 활성화마다 증가 → 클라이언트 재활성화(형상/시각 재구성) 트리거
    UPROPERTY(ReplicatedUsing = OnRep_Activation)
    uint8 ActivationId = 0;

    // 이 액터를 관리하는 풀(없으면 일반 Destroy 로 동작)
    TWeakObjectPtr<ULDHazardPool> OwningPool;

    FTimerHandle DamageTickHandle;
    FTimerHandle LifetimeHandle;
	FTimerHandle TelegraphHandle;

	bool bRowInitialized = false;
};
