// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notify/AnimNotifyState_ComboWindow.h"
#include "AnimNotifyState_ComboWindow.h"
#include "Component/LDAttackComboComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ULDAttackComboComponent* ComboComponent = FindComboComponent(MeshComp))
	{
		ComboComponent->OnComboWindowBegin(Animation);
	}
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ULDAttackComboComponent* ComboComponent = FindComboComponent(MeshComp))
	{
		ComboComponent->OnComboWindowEnd(Animation);
	}
}

ULDAttackComboComponent* UAnimNotifyState_ComboWindow::FindComboComponent(const USkeletalMeshComponent* MeshComp) const
{
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<ULDAttackComboComponent>() : nullptr;
}
