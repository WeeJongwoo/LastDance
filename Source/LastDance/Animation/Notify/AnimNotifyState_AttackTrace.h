// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/AnimSequence.h"
#include "AnimNotifyState_AttackTrace.generated.h"


UENUM(BlueprintType)
enum class EWeaponTraceType : uint8
{
	Line        UMETA(DisplayName = "Line Trace (얇은 검, 도)"),
	Sphere      UMETA(DisplayName = "Sphere Sweep (일반 무기)"),
	Box         UMETA(DisplayName = "Box Sweep (대검, 망치)")
};

UCLASS()
class LASTDANCE_API UAnimNotifyState_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()
	
	
public:
    // 무기 소켓 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    FName StartSocketName = "WeaponStart";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    FName EndSocketName = "WeaponEnd";

    // 충돌 검사 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    EWeaponTraceType TraceType = EWeaponTraceType::Sphere;

    // Sphere Sweep 반경 (TraceType이 Sphere일 때만 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings",
        meta = (EditCondition = "TraceType == EWeaponTraceType::Sphere", EditConditionHides))
    float SphereRadius = 5.0f;

    // Box Sweep 크기 (TraceType이 Box일 때만 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings",
        meta = (EditCondition = "TraceType == EWeaponTraceType::Box", EditConditionHides))
    FVector BoxHalfSize = FVector(10.0f, 10.0f, 10.0f);

    // 애니메이션 프레임당 몇 번 검사할지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    int32 SamplesPerFrame = 2;

    // 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Damage = 10.0f;

    // 애니메이션에서 실제 궤적 샘플링 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    bool bUseAnimationSampling = true;

    // 검의 면적(Blade Surface)을 고려한 검사 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    bool bUseBladeSurface = true;

    // 디버그 시각화 표시 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugTrace = false;

private:
    // 마지막으로 검사한 애니메이션 프레임 번호
    TMap<USkeletalMeshComponent*, int32> LastCheckedFrame;

    // 이전 프레임의 소켓 위치
    TMap<USkeletalMeshComponent*, FVector> PrevStartLocations;
    TMap<USkeletalMeshComponent*, FVector> PrevEndLocations;

    // 이미 히트한 액터 (중복 방지)
    TMap<USkeletalMeshComponent*, TSet<AActor*>> HitActors;

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    // 직선 보간 방식
    void PerformLinearSweepTrace(USkeletalMeshComponent* MeshComp,
        const FVector& PrevStart, const FVector& CurrentStart,
        const FVector& PrevEnd, const FVector& CurrentEnd,
        int32 NumSamples);

    // 애니메이션 샘플링 방식 (정확한 궤적)
    void PerformAnimationSweepTrace(USkeletalMeshComponent* MeshComp,
        UAnimSequence* AnimSequence,
        float PrevTime, float CurrentTime,
        int32 NumSamples);

    // 검의 면적을 고려한 Sweep (대각선 포함)
    void PerformBladeSurfaceSweep(USkeletalMeshComponent* MeshComp,
        const FVector& PrevStart, const FVector& CurrentStart,
        const FVector& PrevEnd, const FVector& CurrentEnd);

    // 단일 충돌 검사 수행 (타입에 따라 Line/Sphere/Box)
    void PerformSingleTrace(UWorld* World, const FVector& Start, const FVector& End,
        const FCollisionQueryParams& QueryParams,
        TArray<FHitResult>& OutHits);

    // 특정 시간의 소켓 위치를 애니메이션에서 가져오기
    bool GetSocketLocationAtTime(USkeletalMeshComponent* MeshComp,
        UAnimSequence* AnimSequence,
        float Time,
        FName SocketName,
        FVector& OutLocation);

    void ProcessHits(USkeletalMeshComponent* MeshComp, const TArray<FHitResult>& HitResults);
    void OnWeaponHit(USkeletalMeshComponent* MeshComp, const FHitResult& Hit);

};
