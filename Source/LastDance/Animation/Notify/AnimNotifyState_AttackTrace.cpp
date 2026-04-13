// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_AttackTrace.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Log/LDLog.h"
#include "Interface/LDCombatInterface.h"

#if ENABLE_DRAW_DEBUG
static TAutoConsoleVariable<int32> CVarShowWeaponTrace(
    TEXT("Game.Debug.ShowWeaponTrace"),
    0,
    TEXT("0: Hide weapon trace debug lines\n1: Show weapon trace debug lines"),
    ECVF_Cheat
);
#endif

void UAnimNotifyState_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp)
        return;

    // 초기 소켓 위치 저장
    FVector StartLocation = MeshComp->GetSocketLocation(StartSocketName);
    FVector EndLocation = MeshComp->GetSocketLocation(EndSocketName);

    PrevStartLocations.Add(MeshComp, StartLocation);
    PrevEndLocations.Add(MeshComp, EndLocation);
    HitActors.Add(MeshComp, TSet<AActor*>());
    LastCheckedFrame.Add(MeshComp, -1);
}

void UAnimNotifyState_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    if (!MeshComp || !MeshComp->GetWorld())
        return;

    UAnimSequence* AnimSequence = Cast<UAnimSequence>(Animation);
    if (!AnimSequence)
    {
        UAnimMontage* AnimMontage = Cast<UAnimMontage>(Animation);
        if (AnimMontage)
        {
            // 몽타주의 첫 번째 슬롯에서 AnimSequence 추출
            if (AnimMontage->SlotAnimTracks.Num() > 0 &&
                AnimMontage->SlotAnimTracks[0].AnimTrack.AnimSegments.Num() > 0)
            {
                AnimSequence = Cast<UAnimSequence>(
                    AnimMontage->SlotAnimTracks[0].AnimTrack.AnimSegments[0].GetAnimReference()
                );
            }

        }
    }
    if (!AnimSequence)
    {
        // AnimSequence를 찾을 수 없으면 기본 동작 (직선 보간)
        LD_LOG(LDLog, Warning, TEXT("%s"), TEXT("Could not get AnimSequence from Animation"));

        FVector CurrentStartLocation = MeshComp->GetSocketLocation(StartSocketName);
        FVector CurrentEndLocation = MeshComp->GetSocketLocation(EndSocketName);

        FVector* PrevStart = PrevStartLocations.Find(MeshComp);
        FVector* PrevEnd = PrevEndLocations.Find(MeshComp);

        if (PrevStart && PrevEnd)
        {
            PerformLinearSweepTrace(MeshComp, *PrevStart, CurrentStartLocation,
                *PrevEnd, CurrentEndLocation, SamplesPerFrame);
        }

        PrevStartLocations[MeshComp] = CurrentStartLocation;
        PrevEndLocations[MeshComp] = CurrentEndLocation;
        return;
    }


    //float CurrentTime = MeshComp->GetPosition();
    float CurrentTime = 0.0f;
    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();

    if (AnimInstance)
    {
        // Montage가 재생 중인지 확인 (특정 슬롯 지정)
        UAnimMontage* CurrentMontage = Cast<UAnimMontage>(Animation);

        if (CurrentMontage && AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            // 현재 몽타주의 정확한 위치 가져오기
            CurrentTime = AnimInstance->Montage_GetPosition(CurrentMontage);

            UE_LOG(LDLog, Verbose, TEXT("Montage Time: %.3f"), CurrentTime);
        }
        else
        {
            // Montage가 안 재생 중이면 일반 애니메이션 시간
            CurrentTime = MeshComp->GetPosition();
        }
    }
    else
    {
        // Fallback: Component에서 직접 가져오기
        CurrentTime = MeshComp->GetPosition();
    }

    // UE 5.4: GetSamplingFrameRate() 사용
    float FrameRate = AnimSequence->GetSamplingFrameRate().AsDecimal();
    int32 CurrentFrame = FMath::FloorToInt(CurrentTime * FrameRate);



    int32* LastFrame = LastCheckedFrame.Find(MeshComp);

    if (LastFrame && CurrentFrame == *LastFrame)
    {
        return; // 같은 애니메이션 프레임이면 검사하지 않음
    }


    int32 SkippedFrames = 1;
    float PrevTime = CurrentTime;

    if (LastFrame && *LastFrame >= 0)
    {
        // 애니메이션이 루프되거나 리셋된 경우 감지
        if (CurrentFrame < *LastFrame)
        {
            // 루프/리셋 감지: 이전 프레임 정보 무시하고 새로 시작
            SkippedFrames = 1;
            PrevTime = CurrentTime;
            UE_LOG(LogTemp, Verbose, TEXT("Animation loop/reset detected in AttackTrace"));
        }
        else
        {
            // 현재 프레임 - 이전 프레임 = 건너뛴 프레임 수
            SkippedFrames = FMath::Max(1, CurrentFrame - *LastFrame);
            PrevTime = (*LastFrame) / FrameRate;
        }
    }


    // 건너뛴 프레임이 많을수록 더 많이 샘플링
    int32 TotalSamples = SkippedFrames * SamplesPerFrame;


    if (bUseAnimationSampling)
    {
        //애니메이션에서 실제 궤적 샘플링
        PerformAnimationSweepTrace(MeshComp, AnimSequence, PrevTime, CurrentTime, TotalSamples);
    }
    else
    {
        //직선 보간
        FVector CurrentStartLocation = MeshComp->GetSocketLocation(StartSocketName);
        FVector CurrentEndLocation = MeshComp->GetSocketLocation(EndSocketName);

        FVector* PrevStart = PrevStartLocations.Find(MeshComp);
        FVector* PrevEnd = PrevEndLocations.Find(MeshComp);

        if (PrevStart && PrevEnd)
        {
            PerformLinearSweepTrace(MeshComp, *PrevStart, CurrentStartLocation,
                *PrevEnd, CurrentEndLocation, TotalSamples);
        }

        PrevStartLocations[MeshComp] = CurrentStartLocation;
        PrevEndLocations[MeshComp] = CurrentEndLocation;
    }

    LastCheckedFrame[MeshComp] = CurrentFrame;
}

void UAnimNotifyState_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    PrevStartLocations.Remove(MeshComp);
    PrevEndLocations.Remove(MeshComp);
    HitActors.Remove(MeshComp);
    LastCheckedFrame.Remove(MeshComp);
}

void UAnimNotifyState_AttackTrace::PerformLinearSweepTrace(USkeletalMeshComponent* MeshComp, const FVector& PrevStart, const FVector& CurrentStart, const FVector& PrevEnd, const FVector& CurrentEnd, int32 NumSamples)
{
    UWorld* World = MeshComp->GetWorld();
    if (!World)
        return;

    AActor* OwnerActor = MeshComp->GetOwner();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerActor);
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;


    for (int32 i = 0; i <= NumSamples; ++i)
    {
        float Alpha = (float)i / (float)NumSamples;

        FVector InterpolatedStart = FMath::Lerp(PrevStart, CurrentStart, Alpha);
        FVector InterpolatedEnd = FMath::Lerp(PrevEnd, CurrentEnd, Alpha);

        TArray<FHitResult> HitResults;
        PerformSingleTrace(World, InterpolatedStart, InterpolatedEnd, QueryParams, HitResults);


#if ENABLE_DRAW_DEBUG
        if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
        {
            DrawDebugLine(World, InterpolatedStart, InterpolatedEnd, FColor::Green, false, 0.5f, 0, 1.0f);
        }
#endif

        ProcessHits(MeshComp, HitResults);
    }
}

void UAnimNotifyState_AttackTrace::PerformAnimationSweepTrace(USkeletalMeshComponent* MeshComp, UAnimSequence* AnimSequence, float PrevTime, float CurrentTime, int32 NumSamples)
{
    UWorld* World = MeshComp->GetWorld();
    if (!World || !AnimSequence)
        return;

    AActor* OwnerActor = MeshComp->GetOwner();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerActor);
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;

    FVector PrevStart, PrevEnd;
    bool bHasPrevPos = false;


    // 애니메이션에서 실제 궤적 샘플링
    for (int32 i = 0; i <= NumSamples; ++i)
    {
        float Alpha = (float)i / (float)NumSamples;
        float SampleTime = FMath::Lerp(PrevTime, CurrentTime, Alpha);

        FVector StartLocation, EndLocation;

        if (!GetSocketLocationAtTime(MeshComp, AnimSequence, SampleTime, StartSocketName, StartLocation))
            continue;

        if (!GetSocketLocationAtTime(MeshComp, AnimSequence, SampleTime, EndSocketName, EndLocation))
            continue;

        // 첫 샘플이 아니고 면적 검사가 활성화되어 있으면 대각선도 검사
        if (bUseBladeSurface && bHasPrevPos && i > 0)
        {
            PerformBladeSurfaceSweep(MeshComp, PrevStart, StartLocation, PrevEnd, EndLocation);
        }
        else
        {
            // 일반 충돌 검사
            TArray<FHitResult> HitResults;
            PerformSingleTrace(World, StartLocation, EndLocation, QueryParams, HitResults);

            // 디버그 시각화
#if ENABLE_DRAW_DEBUG
            if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
            {
                DrawDebugLine(World, StartLocation, EndLocation, FColor::Cyan, false, 0.5f, 0, 1.0f);
            }
#endif

            ProcessHits(MeshComp, HitResults);
        }

        PrevStart = StartLocation;
        PrevEnd = EndLocation;
        bHasPrevPos = true;
    }
}

void UAnimNotifyState_AttackTrace::PerformBladeSurfaceSweep(USkeletalMeshComponent* MeshComp, const FVector& PrevStart, const FVector& CurrentStart, const FVector& PrevEnd, const FVector& CurrentEnd)
{
    UWorld* World = MeshComp->GetWorld();
    if (!World)
        return;

    AActor* OwnerActor = MeshComp->GetOwner();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerActor);
    QueryParams.bTraceComplex = false;


    // 1
    TArray<FHitResult> HitResults;
    PerformSingleTrace(World, CurrentStart, CurrentEnd, QueryParams, HitResults);
    ProcessHits(MeshComp, HitResults);

#if ENABLE_DRAW_DEBUG
    if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
    {
        DrawDebugLine(World, CurrentStart, CurrentEnd, FColor::Green, false, 2.0f, 0, 2.0f);
    }
#endif

    // 2
    HitResults.Reset();
    PerformSingleTrace(World, PrevStart, PrevEnd, QueryParams, HitResults);
    ProcessHits(MeshComp, HitResults);

#if ENABLE_DRAW_DEBUG
    if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
    {
        DrawDebugLine(World, PrevStart, PrevEnd, FColor::Yellow, false, 2.0f, 0, 2.0f);
    }
#endif

    // 3
    HitResults.Reset();
    PerformSingleTrace(World, PrevStart, CurrentEnd, QueryParams, HitResults);
    ProcessHits(MeshComp, HitResults);

#if ENABLE_DRAW_DEBUG
    if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
    {
        DrawDebugLine(World, PrevStart, CurrentEnd, FColor::Red, false, 2.0f, 0, 1.5f);
    }
#endif

    // 4
    HitResults.Reset();
    PerformSingleTrace(World, PrevEnd, CurrentStart, QueryParams, HitResults);
    ProcessHits(MeshComp, HitResults);

#if ENABLE_DRAW_DEBUG
    if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
    {
        DrawDebugLine(World, PrevEnd, CurrentStart, FColor::Red, false, 2.0f, 0, 1.5f);
    }
#endif

    // 5
    HitResults.Reset();
    PerformSingleTrace(World, PrevStart, CurrentStart, QueryParams, HitResults);
    ProcessHits(MeshComp, HitResults);

#if ENABLE_DRAW_DEBUG
    if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
    {
        DrawDebugLine(World, PrevStart, CurrentStart, FColor::Red, false, 2.0f, 0, 1.5f);
    }
#endif

    // 6
    HitResults.Reset();
    PerformSingleTrace(World, PrevEnd, CurrentEnd, QueryParams, HitResults);
    ProcessHits(MeshComp, HitResults);

#if ENABLE_DRAW_DEBUG
    if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
    {
        DrawDebugLine(World, PrevEnd, CurrentEnd, FColor::Red, false, 2.0f, 0, 1.5f);
    }
#endif
}

void UAnimNotifyState_AttackTrace::PerformSingleTrace(UWorld* World, const FVector& Start, const FVector& End, const FCollisionQueryParams& QueryParams, TArray<FHitResult>& OutHits)
{
    if (!World)
        return;

    OutHits.Reset();

    switch (TraceType)
    {
    case EWeaponTraceType::Line:
    {
        World->LineTraceMultiByChannel(
            OutHits,
            Start,
            End,
            ECC_GameTraceChannel3,
            QueryParams
        );
    }
    break;

    case EWeaponTraceType::Sphere:
    {
        FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);
        World->SweepMultiByChannel(
            OutHits,
            Start,
            End,
            FQuat::Identity,
            ECC_GameTraceChannel3,
            SphereShape,
            QueryParams
        );
    }
    break;

    case EWeaponTraceType::Box:
    {
        FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxHalfSize);

        FVector Direction = (End - Start).GetSafeNormal();
        FQuat Rotation = FRotationMatrix::MakeFromX(Direction).ToQuat();

        World->SweepMultiByChannel(
            OutHits,
            Start,
            End,
            Rotation,
            ECC_GameTraceChannel3,
            BoxShape,
            QueryParams
        );
    }
    break;
    }
}

bool UAnimNotifyState_AttackTrace::GetSocketLocationAtTime(USkeletalMeshComponent* MeshComp, UAnimSequence* AnimSequence, float Time, FName SocketName, FVector& OutLocation)
{
    if (!MeshComp || !AnimSequence)
        return false;

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
        return false;

    // 애니메이션 시간 범위 검증
    float AnimLength = AnimSequence->GetPlayLength();
    if (Time < 0.0f || Time > AnimLength)
    {
        // 범위를 벗어난 경우 클램핑
        Time = FMath::Clamp(Time, 0.0f, AnimLength);
    }

    // UE 5.4: 임시 포즈 생성
    FMemMark Mark(FMemStack::Get());

    // 포즈 데이터 준비
    FCompactPose TempPose;
    TempPose.SetBoneContainer(&AnimInstance->GetRequiredBones());

    FBlendedCurve TempCurve;
    TempCurve.InitFrom(AnimInstance->GetRequiredBones());

    UE::Anim::FStackAttributeContainer TempAttributes;

    // UE 5.4: 특정 시간의 포즈 추출
    FAnimExtractContext ExtractionContext(Time, false);
    FAnimationPoseData PoseData(TempPose, TempCurve, TempAttributes);

    AnimSequence->GetAnimationPose(PoseData, ExtractionContext);

    // 컴포넌트 스페이스로 변환
    FCSPose<FCompactPose> ComponentSpacePose;
    ComponentSpacePose.InitPose(TempPose);

    // 소켓의 본 인덱스 찾기
    int32 BoneIndex = MeshComp->GetBoneIndex(SocketName);
    if (BoneIndex == INDEX_NONE)
    {
        // 소켓이 본이 아닐 수 있으므로 소켓으로도 시도
        USkeletalMeshSocket const* Socket = MeshComp->GetSocketByName(SocketName);
        if (Socket)
        {
            BoneIndex = MeshComp->GetBoneIndex(Socket->BoneName);
        }

        if (BoneIndex == INDEX_NONE)
            return false;
    }

    FCompactPoseBoneIndex CompactBoneIndex = AnimInstance->GetRequiredBones().MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneIndex));

    if (!CompactBoneIndex.IsValid())
        return false;

    // 컴포넌트 스페이스 트랜스폼 가져오기
    FTransform SocketTransform = ComponentSpacePose.GetComponentSpaceTransform(CompactBoneIndex);

    // 소켓 오프셋 적용 (소켓이 본에서 오프셋되어 있을 수 있음)
    USkeletalMeshSocket const* Socket = MeshComp->GetSocketByName(SocketName);
    if (Socket && Socket->BoneName == MeshComp->GetBoneName(BoneIndex))
    {
        // UE 5.4: GetSocketTransform은 USkeletalMeshComponent만 받음
        FTransform SocketLocalTransform(Socket->RelativeRotation, Socket->RelativeLocation, Socket->RelativeScale);
        SocketTransform = SocketLocalTransform * SocketTransform;
    }

    // 월드 스페이스로 변환
    OutLocation = MeshComp->GetComponentTransform().TransformPosition(SocketTransform.GetLocation());

    return true;
}

void UAnimNotifyState_AttackTrace::ProcessHits(USkeletalMeshComponent* MeshComp, const TArray<FHitResult>& HitResults)
{
    TSet<AActor*>* HitActorSet = HitActors.Find(MeshComp);
    if (!HitActorSet)
        return;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();

        // 유효한 액터이고 처음 히트한 경우
        if (HitActor && !HitActorSet->Contains(HitActor))
        {
            // 중복 방지를 위해 기록
            HitActorSet->Add(HitActor);

            // 데미지 적용
            OnWeaponHit(MeshComp, Hit);
        }
    }
}

void UAnimNotifyState_AttackTrace::OnWeaponHit(USkeletalMeshComponent* MeshComp, const FHitResult& Hit)
{
    AActor* HitActor = Hit.GetActor();
    if (!HitActor)
        return;

    AActor* OwnerActor = MeshComp->GetOwner();

    // 데미지 적용
    UGameplayStatics::ApplyDamage(
        HitActor,
        Damage,
        OwnerActor ? OwnerActor->GetInstigatorController() : nullptr,
        OwnerActor,
        UDamageType::StaticClass()
    );

    // 디버그 시각화
#if ENABLE_DRAW_DEBUG
    if (bShowDebugTrace || CVarShowWeaponTrace.GetValueOnGameThread() > 0)
    {
        if (UWorld* World = MeshComp->GetWorld())
        {
            DrawDebugSphere(World, Hit.ImpactPoint, 15.0f, 12, FColor::Red, false, 2.0f);
        }
    }
#endif

    UE_LOG(LogTemp, Log, TEXT("Weapon Hit: %s at %s"), *HitActor->GetName(), *Hit.ImpactPoint.ToString());
}
