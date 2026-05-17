// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LDInGameMenuWidget.h"
#include "Components/Button.h"
#include "Player/LDPlayerController.h"

void ULDInGameMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &ULDInGameMenuWidget::ResumeClicked);
    }
    if (ExitButton)
    {
        ExitButton->OnClicked.AddDynamic(this, &ULDInGameMenuWidget::ExitClicked);
    }
}

void ULDInGameMenuWidget::ResumeClicked()
{
    if (ALDPlayerController* PC = Cast<ALDPlayerController>(GetOwningPlayer()))
    {
        PC->ToggleInGameMenu();
    }
}

void ULDInGameMenuWidget::ExitClicked()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    PC->ClientTravel(TEXT("/Game/Maps/MainMenu"), TRAVEL_Absolute);
}
