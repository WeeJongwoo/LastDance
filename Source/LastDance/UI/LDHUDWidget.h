// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LDHUDWidget.generated.h"


class ULDHpBarWidget;
/**
 * 
 */
UCLASS()
class LASTDANCE_API ULDHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	ULDHUDWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	void UpdateHPBar(float CurrentHP, const float MaxHP);

	virtual void NativeConstruct() override;

	void BindToStatComponent(APawn* Pawn);
	
protected:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<ULDHpBarWidget> WBP_HPBar;
	
};
