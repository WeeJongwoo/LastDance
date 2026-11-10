#include "LDComboGraphNode.h"
#include "Animation/AnimMontage.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "LDComboGraphSchema.h"
#include "Types/LDCombatTypes.h"

#define LOCTEXT_NAMESPACE "LDComboGraphNode"

namespace
{
	const FName ComboPinCategory(TEXT("LDCombo"));
	const FName ComboInputPinName(TEXT("In"));
}

bool ULDComboGraphNode::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const
{
	return Schema && Schema->IsA<ULDComboGraphSchema>();
}

void ULDComboGraphNode::CreateCommandPins()
{
	const UEnum* CommandEnum = StaticEnum<ELDAttackCommand>();
	const int32 First = static_cast<int32>(ELDAttackCommand::None) + 1;
	const int32 Last = static_cast<int32>(ELDAttackCommand::MAX);

	for (int32 Value = First; Value < Last; ++Value)
	{
		// 핀 이름은 enum 이름으로 고정, 표시 이름만 DisplayName 사용
		UEdGraphPin* Pin = CreatePin(EGPD_Output, ComboPinCategory, FName(*CommandEnum->GetNameStringByValue(Value)));
		Pin->PinFriendlyName = CommandEnum->GetDisplayNameTextByValue(Value);
	}
}

void ULDComboEntryNode::AllocateDefaultPins()
{
	CreateCommandPins();
}

FText ULDComboEntryNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("EntryTitle", "Entry");
}

void ULDComboActionNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, ComboPinCategory, ComboInputPinName);
	CreateCommandPins();
}

FText ULDComboActionNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return ActionName.IsNone() ? LOCTEXT("NewActionTitle", "New Action") : FText::FromName(ActionName);
}

void ULDComboActionNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	// 출력 핀에서 끌어온 경우에만 새 노드의 입력에 연결
	if (FromPin && FromPin->Direction == EGPD_Output)
	{
		GetSchema()->TryCreateConnection(FromPin, FindPinChecked(ComboInputPinName));
	}
}

#undef LOCTEXT_NAMESPACE
