#include "LDComboGraphSchema.h"
#include "EdGraph/EdGraph.h"
#include "LDComboGraphNode.h"

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

#undef LOCTEXT_NAMESPACE
