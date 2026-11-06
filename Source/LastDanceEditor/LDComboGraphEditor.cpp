#include "LDComboGraphEditor.h"
#include "Data/LDComboDataAsset.h"
#include "EdGraph/EdGraph.h"
#include "GraphEditor.h"
#include "IDetailsView.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "LDComboGraphSchema.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "LDComboGraphEditor"

namespace LDComboGraphEditorIds
{
	static const FName App(TEXT("LDComboGraphEditorApp"));
	static const FName GraphTab(TEXT("LDComboGraphEditor_Graph"));
	static const FName DetailsTab(TEXT("LDComboGraphEditor_Details"));
}

void FLDComboGraphEditor::InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULDComboDataAsset* InComboData)
{
	ComboData = InComboData;

	// 기존 에셋에는 그래프가 없으므로 처음 열 때 생성
	if (!ComboData->ComboGraph)
	{
		ComboData->ComboGraph = FBlueprintEditorUtils::CreateNewGraph(ComboData, TEXT("ComboGraph"), UEdGraph::StaticClass(), ULDComboGraphSchema::StaticClass());
		ComboData->ComboGraph->GetSchema()->CreateDefaultNodesForGraph(*ComboData->ComboGraph);
	}

	SGraphEditor::FGraphEditorEvents GraphEvents;
	GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FLDComboGraphEditor::OnGraphSelectionChanged);

	GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(ComboData->ComboGraph)
		.GraphEvents(GraphEvents);

	// InitAssetEditor가 탭을 만들기 때문에 위젯을 먼저 생성
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsArgs;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditor.CreateDetailView(DetailsArgs);
	DetailsView->SetObject(ComboData);

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("LDComboGraphEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.7f)
				->AddTab(LDComboGraphEditorIds::GraphTab, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.3f)
				->AddTab(LDComboGraphEditorIds::DetailsTab, ETabState::OpenedTab)
			)
		);

	InitAssetEditor(Mode, InitToolkitHost, LDComboGraphEditorIds::App, Layout, true, true, ComboData);
}

void FLDComboGraphEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu", "Combo Graph Editor"));

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(LDComboGraphEditorIds::GraphTab, FOnSpawnTab::CreateSP(this, &FLDComboGraphEditor::SpawnGraphTab))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(LDComboGraphEditorIds::DetailsTab, FOnSpawnTab::CreateSP(this, &FLDComboGraphEditor::SpawnDetailsTab))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FLDComboGraphEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(LDComboGraphEditorIds::GraphTab);
	InTabManager->UnregisterTabSpawner(LDComboGraphEditorIds::DetailsTab);
}

FName FLDComboGraphEditor::GetToolkitFName() const
{
	return FName(TEXT("LDComboGraphEditor"));
}

FText FLDComboGraphEditor::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "Combo Graph Editor");
}

FString FLDComboGraphEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("ComboGraph ");
}

FLinearColor FLDComboGraphEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

TSharedRef<SDockTab> FLDComboGraphEditor::SpawnGraphTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		[
			GraphEditor.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLDComboGraphEditor::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		[
			DetailsView.ToSharedRef()
		];
}

void FLDComboGraphEditor::OnGraphSelectionChanged(const TSet<UObject*>& Selection)
{
	if (Selection.Num() == 1)
	{
		DetailsView->SetObject(*Selection.CreateConstIterator());
	}
	else
	{
		DetailsView->SetObject(ComboData);
	}
}

#undef LOCTEXT_NAMESPACE
