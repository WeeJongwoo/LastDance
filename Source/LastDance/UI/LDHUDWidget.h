// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LDHUDWidget.generated.h"


class ULDHpBarWidget;
class ALDBossCharacter;
class ULDBossHpBarWidget;
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

	void ShowBossBar(ALDBossCharacter* Boss);
	void HideBossBar();
	
protected:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<ULDHpBarWidget> WBP_HPBar;
	

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	TObjectPtr<ULDBossHpBarWidget> WBP_BossHpBar;
	
};
