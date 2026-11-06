#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class IDetailsView;
class SDockTab;
class SGraphEditor;
class ULDComboDataAsset;

class FLDComboGraphEditor : public FAssetEditorToolkit
{
public:
	void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULDComboDataAsset* InComboData);

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

private:
	TSharedRef<SDockTab> SpawnGraphTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnDetailsTab(const FSpawnTabArgs& Args);
	void OnGraphSelectionChanged(const TSet<UObject*>& Selection);

	ULDComboDataAsset* ComboData = nullptr;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SGraphEditor> GraphEditor;
};
