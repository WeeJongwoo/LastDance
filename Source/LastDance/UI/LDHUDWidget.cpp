// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LDHUDWidget.h"
#include "LDHpBarWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Log/LDLog.h"
#include "Interface/LDStatInterface.h"

ULDHUDWidget::ULDHUDWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void ULDHUDWidget::UpdateHPBar(float CurrentHP, float MaxHP)
{
	if (WBP_HPBar)
	{
		LD_LOG(LDLog, Log, TEXT("Current HP: %f / MaxHP: %f"), CurrentHP, MaxHP);

		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(WBP_HPBar->Slot);
		if (CanvasSlot)
		{
			float HPBarLength = MaxHP * 4;
			float HPBarHeight = CanvasSlot->GetSize().Y;

			CanvasSlot->SetSize(FVector2D(HPBarLength, HPBarHeight));
		}
		WBP_HPBar->SetHPPercent(CurrentHP / MaxHP);
	}
}

void ULDHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToStatComponent(GetOwningPlayerPawn());
}

void ULDHUDWidget::BindToStatComponent(APawn* Pawn)
{
	if (!Pawn)
	{
		return;
	}

	ILDStatInterface* StatInterface = Cast<ILDStatInterface>(Pawn);
	if (StatInterface)
	{
		ULDStatComponent* StatComponent = StatInterface->GetStatComponent();
		if (StatComponent)
		{
			StatComponent->OnCurrentHPChanged.RemoveDynamic(this, &ULDHUDWidget::UpdateHPBar);
			StatComponent->OnCurrentHPChanged.AddDynamic(this, &ULDHUDWidget::UpdateHPBar);
			UpdateHPBar(StatComponent->GetCurrentHP(), StatComponent->GetMaxHP());
		}
	}
}
