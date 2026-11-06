#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "LDComboDataFactory.generated.h"

UCLASS()
class ULDComboDataFactory : public UFactory
{
	GENERATED_BODY()

public:
	ULDComboDataFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
