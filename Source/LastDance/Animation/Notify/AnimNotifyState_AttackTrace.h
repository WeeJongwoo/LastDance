// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/AnimSequence.h"
#include "Types/LDCombatTypes.h"
#include "AnimNotifyState_AttackTrace.generated.h"


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

    // 검의 면적(Blade Surface)을 고려한 검사 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    bool bUseBladeSurface = true;

    // 디버그 시각화 표시 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugTrace = false;

private:
    // 마지막으로 검사한 애니메이션 프레임 번호
    TMap<USkeletalMeshComponent*, int32> LastCheckedFrame;

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:

    // 특정 시간의 소켓 위치를 애니메이션에서 가져오기
    bool GetSocketLocationAtTime(USkeletalMeshComponent* MeshComp,
        UAnimSequence* AnimSequence,
        float Time,
        FName SocketName,
        FVector& OutLocation);
};
