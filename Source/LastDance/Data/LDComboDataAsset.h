// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/LDCombatTypes.h"
#include "LDComboDataAsset.generated.h"

class UEdGraph;

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
    // 그래프에서 자동 컴파일되는 런타임 데이터. 디테일 패널에는 표시하지 않음
    // 비전투 상태에서 입력 시 시작할 액션
    UPROPERTY(BlueprintReadOnly, Category = "Combo")
    TArray<FLDComboNext> StartActions;

    UPROPERTY(BlueprintReadOnly, Category = "Combo")
    TArray<FLDComboAction> Actions;

#if WITH_EDITORONLY_DATA
    // 에디터 전용 편집 원본. 쿠킹된 빌드에는 포함되지 않음
    UPROPERTY()
    TObjectPtr<UEdGraph> ComboGraph;
#endif
};
