// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_CounterStart.h"
#include "Character/LDBossCharacter.h"

void UAnimNotify_CounterStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp)
	{
		return;
	}
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (ALDBossCharacter* Boss = Cast<ALDBossCharacter>(MeshComp->GetOwner()))
	{
		Boss->SetCounterHitEnabled();
	}
}
