// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LDHpBarWidget.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class LASTDANCE_API ULDHpBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	ULDHpBarWidget(const FObjectInitializer& ObjectInitializer);

	void SetHPPercent(float Percent);
	
protected:

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	TObjectPtr<UProgressBar> HPBar;
	
};
