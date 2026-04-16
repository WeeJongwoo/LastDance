// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LDHpBarWidget.h"
#include "Components/ProgressBar.h"

ULDHpBarWidget::ULDHpBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void ULDHpBarWidget::SetHPPercent(float Percent)
{
	if (HPBar)
	{
		HPBar->SetPercent(Percent);
	}
}