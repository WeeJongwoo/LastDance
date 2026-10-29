// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LDComboDataAsset.generated.h"

struct FLDComboNext;
struct FLDComboAction;
/**
 * 
 */
UCLASS()
class LASTDANCE_API ULDComboDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    int32 FindActionIndex(FName ActionName) const;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

    // 디테일 패널에 버튼으로 표시됨
    UFUNCTION(CallInEditor, Category = "Combo|Debug")
    void PrintComboTree();

    UFUNCTION()
    TArray<FName> GetActionNameOptions() const;

private:
    void AppendTree(const FLDComboNext& Next, const FString& Prefix, bool bIsLast, TSet<FName>& Path, FString& Out) const;

public:
    // 비전투 상태에서 입력 시 시작할 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (TitleProperty = "{Command} → {ToAction}"))
    TArray<FLDComboNext> StartActions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (TitleProperty = "ActionName"))
    TArray<FLDComboAction> Actions;
	
	
};
