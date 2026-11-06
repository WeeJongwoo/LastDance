#include "LDComboDataFactory.h"
#include "Data/LDComboDataAsset.h"

ULDComboDataFactory::ULDComboDataFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = ULDComboDataAsset::StaticClass();
}

UObject* ULDComboDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<ULDComboDataAsset>(InParent, InClass, InName, Flags);
}
