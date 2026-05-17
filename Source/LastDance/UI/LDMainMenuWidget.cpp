// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LDMainMenuWidget.h"
#include "Components/Button.h"
#include "Settings/LDNetworkSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void ULDMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ULDMainMenuWidget::StartButtonClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &ULDMainMenuWidget::QuitButtonClicked);
	}
}

void ULDMainMenuWidget::StartButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	const FString Addr = GetDefault<ULDNetworkSettings>()->ServerAddress;
	PC->ClientTravel(Addr, TRAVEL_Absolute);
}

void ULDMainMenuWidget::QuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}