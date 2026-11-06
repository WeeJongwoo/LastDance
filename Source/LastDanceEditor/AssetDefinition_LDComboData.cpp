#include "AssetDefinition_LDComboData.h"
#include "Data/LDComboDataAsset.h"
#include "LDComboGraphEditor.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_LDComboData"

FText UAssetDefinition_LDComboData::GetAssetDisplayName() const
{
	return LOCTEXT("DisplayName", "Combo Graph");
}

FLinearColor UAssetDefinition_LDComboData::GetAssetColor() const
{
	return FColor(201, 120, 29);
}

TSoftClassPtr<UObject> UAssetDefinition_LDComboData::GetAssetClass() const
{
	return ULDComboDataAsset::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_LDComboData::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] = { EAssetCategoryPaths::Gameplay };
	return Categories;
}

EAssetCommandResult UAssetDefinition_LDComboData::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	for (ULDComboDataAsset* ComboData : OpenArgs.LoadObjects<ULDComboDataAsset>())
	{
		TSharedRef<FLDComboGraphEditor> Editor = MakeShared<FLDComboGraphEditor>();
		Editor->InitEditor(OpenArgs.GetToolkitMode(), OpenArgs.ToolkitHost, ComboData);
	}

	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE
