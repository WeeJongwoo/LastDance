#include "LDComboGraphEditor.h"
#include "Data/LDComboDataAsset.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphUtilities.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "GraphEditor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailsView.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "LDComboGraphNode.h"
#include "LDComboGraphSchema.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "UObject/UnrealType.h"
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

	// 그래프가 바뀔 때마다 Actions/StartActions로 다시 컴파일
	ComboData->ComboGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FLDComboGraphEditor::OnComboGraphChanged));
	CompileGraph();

	SGraphEditor::FGraphEditorEvents GraphEvents;
	GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FLDComboGraphEditor::OnGraphSelectionChanged);

	BindGraphCommands();

	GraphEditor = SNew(SGraphEditor)
		.AdditionalCommands(GraphCommands)
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

void FLDComboGraphEditor::OnComboGraphChanged(const FEdGraphEditAction& Action)
{
	CompileGraph();
}

void FLDComboGraphEditor::CompileGraph()
{
	TArray<FLDComboNext> NewStartActions;
	TArray<FLDComboAction> NewActions;

	// 노드 순서 = Actions 순서 (네트워크 ID)
	for (const UEdGraphNode* Node : ComboData->ComboGraph->Nodes)
	{
		if (const ULDComboActionNode* ActionNode = Cast<ULDComboActionNode>(Node))
		{
			FLDComboAction& Action = NewActions.AddDefaulted_GetRef();
			Action.ActionName = ActionNode->ActionName;
			Action.Montage = ActionNode->Montage;
			Action.DamageMultiplier = ActionNode->DamageMultiplier;
			ActionNode->AppendNextActions(Action.NextActions);
		}
		else if (const ULDComboEntryNode* EntryNode = Cast<ULDComboEntryNode>(Node))
		{
			EntryNode->AppendNextActions(NewStartActions);
		}
	}

	// 결과가 같으면 에셋을 수정 상태로 만들지 않음
	const UClass* DataClass = ULDComboDataAsset::StaticClass();
	const FProperty* StartProperty = FindFProperty<FProperty>(DataClass, GET_MEMBER_NAME_CHECKED(ULDComboDataAsset, StartActions));
	const FProperty* ActionsProperty = FindFProperty<FProperty>(DataClass, GET_MEMBER_NAME_CHECKED(ULDComboDataAsset, Actions));
	if (StartProperty->Identical(&ComboData->StartActions, &NewStartActions) &&
		ActionsProperty->Identical(&ComboData->Actions, &NewActions))
	{
		return;
	}

	ComboData->Modify();
	ComboData->StartActions = MoveTemp(NewStartActions);
	ComboData->Actions = MoveTemp(NewActions);
}

void FLDComboGraphEditor::BindGraphCommands()
{
	GraphCommands = MakeShared<FUICommandList>();

	GraphCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &FLDComboGraphEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FLDComboGraphEditor::CanDeleteNodes));

	GraphCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateSP(this, &FLDComboGraphEditor::CopySelectedNodes),
		FCanExecuteAction::CreateSP(this, &FLDComboGraphEditor::CanCopyNodes));

	GraphCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateSP(this, &FLDComboGraphEditor::CutSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FLDComboGraphEditor::CanCopyNodes));

	GraphCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateSP(this, &FLDComboGraphEditor::PasteNodes),
		FCanExecuteAction::CreateSP(this, &FLDComboGraphEditor::CanPasteNodes));

	GraphCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &FLDComboGraphEditor::DuplicateNodes),
		FCanExecuteAction::CreateSP(this, &FLDComboGraphEditor::CanCopyNodes));
}

void FLDComboGraphEditor::DeleteSelectedNodes()
{
	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());
	ComboData->ComboGraph->Modify();

	// ClearSelectionSet이 선택 목록을 비우므로 복사본으로 순회
	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
	GraphEditor->ClearSelectionSet();

	for (UObject* Object : SelectedNodes)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node && Node->CanUserDeleteNode())
		{
			Node->Modify();
			Node->DestroyNode();
		}
	}
}

bool FLDComboGraphEditor::CanDeleteNodes() const
{
	for (UObject* Object : GraphEditor->GetSelectedNodes())
	{
		const UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FLDComboGraphEditor::CopySelectedNodes()
{
	FGraphPanelSelectionSet NodesToCopy;
	for (UObject* Object : GraphEditor->GetSelectedNodes())
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node && Node->CanDuplicateNode())
		{
			Node->PrepareForCopying();
			NodesToCopy.Add(Node);
		}
	}

	FString ExportedText;
	FEdGraphUtilities::ExportNodesToText(NodesToCopy, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FLDComboGraphEditor::CanCopyNodes() const
{
	for (UObject* Object : GraphEditor->GetSelectedNodes())
	{
		const UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FLDComboGraphEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedNodes();
}

void FLDComboGraphEditor::PasteNodes()
{
	const FVector2D PasteLocation = GraphEditor->GetPasteLocation();
	UEdGraph* Graph = ComboData->ComboGraph;

	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
	Graph->Modify();
	GraphEditor->ClearSelectionSet();

	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(Graph, TextToImport, PastedNodes);

	if (PastedNodes.IsEmpty())
	{
		return;
	}

	// 노드끼리의 상대 위치를 유지하기 위해 평균 위치를 기준으로 이동
	FVector2D AveragePosition = FVector2D::ZeroVector;
	for (const UEdGraphNode* Node : PastedNodes)
	{
		AveragePosition += FVector2D(Node->NodePosX, Node->NodePosY);
	}
	AveragePosition /= PastedNodes.Num();

	for (UEdGraphNode* Node : PastedNodes)
	{
		Node->NodePosX = static_cast<int32>(Node->NodePosX - AveragePosition.X + PasteLocation.X);
		Node->NodePosY = static_cast<int32>(Node->NodePosY - AveragePosition.Y + PasteLocation.Y);
		Node->SnapToGrid(16);
		Node->CreateNewGuid();
		GraphEditor->SetNodeSelection(Node, true);
	}

	GraphEditor->NotifyGraphChanged();
}

bool FLDComboGraphEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(ComboData->ComboGraph, ClipboardContent);
}

void FLDComboGraphEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

void FLDComboGraphEditor::PostUndo(bool bSuccess)
{
	if (bSuccess)
	{
		// 되돌리면서 사라진 노드를 선택 목록에 남기지 않음
		GraphEditor->ClearSelectionSet();
		GraphEditor->NotifyGraphChanged();
	}
}

void FLDComboGraphEditor::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

#undef LOCTEXT_NAMESPACE
