// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ULDHUDWidget;
class ALDBossCharacter;

/**
 * 
 */
UCLASS()
class LASTDANCE_API ALDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ALDPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(Client, Reliable)
	void ClientRPC_NotifyRecognizedByBoss(ALDBossCharacter* Boss);

	UFUNCTION(Client, Reliable)
	void ClientRPC_NotifyBossDefeated(ALDBossCharacter* Boss);

	void ToggleInGameMenu();

protected:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void AcknowledgePossession(class APawn* P) override;

	UFUNCTION()
	void UpdateHP(float NewHP, float MaxHP);

	void ApplyPendingBossToHUD();

	void SetInGameMenuVisible(bool bVisible);
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> ControllerMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ToggleMenuAction;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULDHUDWidget> HUDClass;


	TObjectPtr<ULDHUDWidget> HUDWidget;

	uint8 bRegisteredWithGameMode : 1;

	UPROPERTY()
	TObjectPtr<ALDBossCharacter> PendingBoss;


	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ULDInGameMenuWidget> InGameMenuClass;

	TObjectPtr<class ULDInGameMenuWidget> InGameMenuWidget;

};
