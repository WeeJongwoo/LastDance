// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_AttackTrace.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Log/LDLog.h"
#include "Interface/LDCombatInterface.h"

void UAnimNotifyState_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}
		
	LastCheckedFrame.Add(MeshComp, -1);

	// CombatComponent에 공격 트레이스 시작 알림
	AActor* Owner = MeshComp->GetOwner();
	if (ILDCombatInterface* CombatInterface = Cast<ILDCombatInterface>(Owner))
	{
		if (ULDCombatComponent* CombatComp = CombatInterface->GetCombatComponent())
		{
			CombatComp->BeginAttackTrace();
		}
	}
}

void UAnimNotifyState_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetWorld())
	{
		return;
	}

	// CombatComponent 획득
	AActor* Owner = MeshComp->GetOwner();
	APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	ILDCombatInterface* CombatInterface = Cast<ILDCombatInterface>(Owner);
	if (!CombatInterface)
	{
		return;
	}

	ULDCombatComponent* CombatComp = CombatInterface->GetCombatComponent();
	if (!CombatComp)
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// AnimSequence 추출
	UAnimSequence* AnimSequence = Cast<UAnimSequence>(Animation);
	if (!AnimSequence)
	{
		UAnimMontage* AnimMontage = Cast<UAnimMontage>(Animation);
		if (AnimMontage && AnimMontage->SlotAnimTracks.Num() > 0)
		{
			const FAnimTrack& AnimTrack = AnimMontage->SlotAnimTracks[0].AnimTrack;

			// 현재 몽타주 재생 위치로 해당 세그먼트 찾기
			float MontagePosition = AnimInstance->Montage_GetPosition(AnimMontage);

			if (const FAnimSegment* Segment = AnimTrack.GetSegmentAtTime(MontagePosition))
			{
				AnimSequence = Cast<UAnimSequence>(Segment->GetAnimReference());
			}
		}
	}
	if (!AnimSequence)
	{
		LD_LOG(LDLog, Warning, TEXT("%s"), TEXT("Could not get AnimSequence from Animation, skipping trace"));
		return;
	}

	// 현재 시간 계산
	float CurrentTime = 0.0f;

	UAnimMontage* CurrentMontage = Cast<UAnimMontage>(Animation);

	if (CurrentMontage && AnimInstance->Montage_IsPlaying(CurrentMontage))
	{
		CurrentTime = AnimInstance->Montage_GetPosition(CurrentMontage);
		UE_LOG(LDLog, Verbose, TEXT("Montage Time: %.3f"), CurrentTime);
	}
	else
	{
		CurrentTime = MeshComp->GetPosition();
	}

	// 프레임 계산
	float FrameRate = AnimSequence->GetSamplingFrameRate().AsDecimal();
	int32 CurrentFrame = FMath::FloorToInt(CurrentTime * FrameRate);

	int32* LastFrame = LastCheckedFrame.Find(MeshComp);

	if (LastFrame && CurrentFrame == *LastFrame)
	{
		return;
	}

	int32 SkippedFrames = 1;
	float PrevTime = CurrentTime;

	if (LastFrame && *LastFrame >= 0)
	{
		if (CurrentFrame < *LastFrame)
		{
			SkippedFrames = 1;
			PrevTime = CurrentTime;
			UE_LOG(LogTemp, Verbose, TEXT("Animation loop/reset detected in AttackTrace"));
		}
		else
		{
			SkippedFrames = FMath::Max(1, CurrentFrame - *LastFrame);
			PrevTime = (*LastFrame) / FrameRate;
		}
	}

	int32 TotalSamples = SkippedFrames * SamplesPerFrame;

	// 애니메이션에서 소켓 위치 샘플링하여 배열 생성
	TArray<FWeaponTraceSample> Samples;
	Samples.Reserve(TotalSamples + 1);

	for (int32 i = 0; i <= TotalSamples; ++i)
	{
		float Alpha = (float)i / (float)TotalSamples;
		float SampleTime = FMath::Lerp(PrevTime, CurrentTime, Alpha);

		FWeaponTraceSample Sample;

		if (!GetSocketLocationAtTime(MeshComp, AnimSequence, SampleTime, StartSocketName, Sample.StartLocation))
			continue;

		if (!GetSocketLocationAtTime(MeshComp, AnimSequence, SampleTime, EndSocketName, Sample.EndLocation))
			continue;

		Samples.Add(Sample);
	}

	// CombatComponent에 트레이스 실행 위임
	FWeaponTraceParams Params;
	Params.TraceType = TraceType;
	Params.SphereRadius = SphereRadius;
	Params.BoxHalfSize = BoxHalfSize;
	Params.Damage = Damage;
	Params.bUseBladeSurface = bUseBladeSurface;
	Params.bShowDebugTrace = bShowDebugTrace;

	CombatComp->ExecuteAttackTraceSamples(Samples, Params);

	LastCheckedFrame[MeshComp] = CurrentFrame;
}

void UAnimNotifyState_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	LastCheckedFrame.Remove(MeshComp);

	// CombatComponent에 공격 트레이스 종료 알림
	AActor* Owner = MeshComp->GetOwner();
	if (ILDCombatInterface* CombatInterface = Cast<ILDCombatInterface>(Owner))
	{
		if (ULDCombatComponent* CombatComp = CombatInterface->GetCombatComponent())
		{
			CombatComp->EndAttackTrace();
		}
	}
}

bool UAnimNotifyState_AttackTrace::GetSocketLocationAtTime(USkeletalMeshComponent* MeshComp, UAnimSequence* AnimSequence, float Time, FName SocketName, FVector& OutLocation)
{
	if (!MeshComp || !AnimSequence)
		return false;

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return false;

	float AnimLength = AnimSequence->GetPlayLength();
	if (Time < 0.0f || Time > AnimLength)
	{
		Time = FMath::Clamp(Time, 0.0f, AnimLength);
	}

	FMemMark Mark(FMemStack::Get());

	FCompactPose TempPose;
	TempPose.SetBoneContainer(&AnimInstance->GetRequiredBones());

	FBlendedCurve TempCurve;
	TempCurve.InitFrom(AnimInstance->GetRequiredBones());

	UE::Anim::FStackAttributeContainer TempAttributes;

	FAnimExtractContext ExtractionContext(Time, false);
	FAnimationPoseData PoseData(TempPose, TempCurve, TempAttributes);

	AnimSequence->GetAnimationPose(PoseData, ExtractionContext);

	FCSPose<FCompactPose> ComponentSpacePose;
	ComponentSpacePose.InitPose(TempPose);

	int32 BoneIndex = MeshComp->GetBoneIndex(SocketName);
	if (BoneIndex == INDEX_NONE)
	{
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

	FTransform SocketTransform = ComponentSpacePose.GetComponentSpaceTransform(CompactBoneIndex);

	USkeletalMeshSocket const* Socket = MeshComp->GetSocketByName(SocketName);
	if (Socket && Socket->BoneName == MeshComp->GetBoneName(BoneIndex))
	{
		FTransform SocketLocalTransform(Socket->RelativeRotation, Socket->RelativeLocation, Socket->RelativeScale);
		SocketTransform = SocketLocalTransform * SocketTransform;
	}

	OutLocation = MeshComp->GetComponentTransform().TransformPosition(SocketTransform.GetLocation());

	return true;
}