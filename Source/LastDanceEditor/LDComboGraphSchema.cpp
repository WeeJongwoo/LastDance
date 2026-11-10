#include "LDComboGraphSchema.h"
#include "Animation/AnimMontage.h"
#include "AssetRegistry/AssetData.h"
#include "EdGraph/EdGraph.h"
#include "LDComboGraphNode.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "LDComboGraphSchema"

void ULDComboGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<ULDComboEntryNode> NodeCreator(Graph);
	NodeCreator.CreateNode();
	NodeCreator.Finalize();
}

void ULDComboGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	TSharedPtr<FEdGraphSchemaAction_NewNode> NewAction = MakeShared<FEdGraphSchemaAction_NewNode>(
		LOCTEXT("ComboCategory", "Combo"),
		LOCTEXT("NewAction", "New Action"),
		LOCTEXT("NewActionTooltip", "Add a combo action"),
		0);

	NewAction->NodeTemplate = ContextMenuBuilder.CreateTemplateNode<ULDComboActionNode>();
	ContextMenuBuilder.AddAction(NewAction);
}

const FPinConnectionResponse ULDComboGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SelfLink", "A node cannot link to itself"));
	}

	if (A->Direction == B->Direction)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SameDirection", "Connect an output to an input"));
	}

	// 출력 핀은 연결 하나만 유지. 입력 핀은 여러 연결 허용
	const ECanCreateConnectionResponse Response = A->Direction == EGPD_Output ? CONNECT_RESPONSE_BREAK_OTHERS_A : CONNECT_RESPONSE_BREAK_OTHERS_B;
	return FPinConnectionResponse(Response, LOCTEXT("ReplaceLink", "Connect"));
}

void ULDComboGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const
{
	const FScopedTransaction Transaction(LOCTEXT("DropMontage", "Add Combo Action"));

	int32 NodePosY = static_cast<int32>(GraphPosition.Y);
	for (const FAssetData& Asset : Assets)
	{
		UAnimMontage* Montage = Cast<UAnimMontage>(Asset.GetAsset());
		if (!Montage)
		{
			continue;
		}

		// 몽타주가 있을 때만 기록해서, 다른 에셋을 놓으면 에셋이 변경되지 않음
		Graph->Modify();

		FGraphNodeCreator<ULDComboActionNode> NodeCreator(*Graph);
		ULDComboActionNode* Node = NodeCreator.CreateUserInvokedNode();
		Node->ActionName = Asset.AssetName;
		Node->Montage = Montage;
		Node->NodePosX = static_cast<int32>(GraphPosition.X);
		Node->NodePosY = NodePosY;
		NodeCreator.Finalize();

		// 여러 개를 놓으면 세로로 배치
		NodePosY += 160;
	}
}

void ULDComboGraphSchema::GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const
{
	OutOkIcon = Assets.ContainsByPredicate([](const FAssetData& Asset)
		{
			return Asset.IsInstanceOf(UAnimMontage::StaticClass());
		});

	OutTooltipText = OutOkIcon ? TEXT("Add a combo action") : TEXT("Drop an animation montage");
}

#undef LOCTEXT_NAMESPACE
