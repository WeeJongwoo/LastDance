// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LDBossHpBarWidget.generated.h"

class ALDBossCharacter;
class UProgressBar;
class UTextBlock;

UCLASS()
class LASTDANCE_API ULDBossHpBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void BindToBoss(ALDBossCharacter* Boss);
	void Unbind();


protected:

	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleHPChanged(float CurrentHP, float MaxHP);

	UFUNCTION()
	void HandleBossDeath();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BossHpBar;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> BossName;

	// 바인딩 해제용 약참조
	UPROPERTY()
	TWeakObjectPtr<ALDBossCharacter> BoundBoss;
	
	
};
