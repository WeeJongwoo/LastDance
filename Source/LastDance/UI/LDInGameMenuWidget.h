// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LDInGameMenuWidget.generated.h"

class UButton;


UCLASS()
class LASTDANCE_API ULDInGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ResumeButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ExitButton;

    UFUNCTION()
    void ResumeClicked();

    UFUNCTION()
    void ExitClicked();
	
};
