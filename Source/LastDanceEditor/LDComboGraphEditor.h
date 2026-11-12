#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "EditorUndoClient.h"

class IDetailsView;
class SDockTab;
class SGraphEditor;
class ULDComboDataAsset;
struct FEdGraphEditAction;

class FLDComboGraphEditor : public FAssetEditorToolkit, public FSelfRegisteringEditorUndoClient
{
public:
	void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULDComboDataAsset* InComboData);

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

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
	void OnComboGraphChanged(const FEdGraphEditAction& Action);

	// 노드마다 오류/경고를 계산해서 노드 위에 표시
	void UpdateNodeErrors();

	// 그래프를 Actions/StartActions로 변환. 결과가 같으면 에셋을 수정하지 않음
	void CompileGraph();

	void BindGraphCommands();

	void DeleteSelectedNodes();
	bool CanDeleteNodes() const;
	void CopySelectedNodes();
	bool CanCopyNodes() const;
	void CutSelectedNodes();
	void PasteNodes();
	bool CanPasteNodes() const;
	void DuplicateNodes();

	ULDComboDataAsset* ComboData = nullptr;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<FUICommandList> GraphCommands;
};
