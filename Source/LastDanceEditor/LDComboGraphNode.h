#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "Types/LDCombatTypes.h"
#include "LDComboGraphNode.generated.h"

class UAnimMontage;

UCLASS(Abstract)
class ULDComboGraphNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	// 콤보 그래프에서만 생성, 붙여넣기 가능
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	// 출력 핀의 연결을 FLDComboNext 목록으로 변환
	void AppendNextActions(TArray<FLDComboNext>& OutNexts) const;

protected:
	// ELDAttackCommand의 커맨드마다 출력 핀 생성
	void CreateCommandPins();
};

UCLASS()
class ULDComboEntryNode : public ULDComboGraphNode
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
};

UCLASS()
class ULDComboActionNode : public ULDComboGraphNode
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditAnywhere, Category = "Combo")
	FName ActionName;

	UPROPERTY(EditAnywhere, Category = "Combo")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, Category = "Combo")
	float DamageMultiplier = 1.0f;
};
