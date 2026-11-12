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

void ULDComboGraphNode::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	// 연결 변경은 그래프 변경 이벤트가 없어서 직접 알림
	GetGraph()->NotifyNodeChanged(this);
}

void ULDComboGraphNode::AppendNextActions(TArray<FLDComboNext>& OutNexts) const
{
	const UEnum* CommandEnum = StaticEnum<ELDAttackCommand>();

	for (const UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction != EGPD_Output || Pin->LinkedTo.IsEmpty())
		{
			continue;
		}

		// 핀 이름(LMB, RMB)을 커맨드 값으로 되돌림
		const int64 Value = CommandEnum->GetValueByName(Pin->PinName);
		const ULDComboActionNode* ToNode = Cast<ULDComboActionNode>(Pin->LinkedTo[0]->GetOwningNode());
		if (Value == INDEX_NONE || !ToNode)
		{
			continue;
		}

		FLDComboNext& Next = OutNexts.AddDefaulted_GetRef();
		Next.Command = static_cast<ELDAttackCommand>(Value);
		Next.ToAction = ToNode->ActionName;
	}
}

void ULDComboGraphNode::ReconstructNode()
{
	TArray<UEdGraphPin*> OldPins = MoveTemp(Pins);
	Pins.Reset();
	AllocateDefaultPins();

	bool bChanged = OldPins.Num() != Pins.Num();
	for (int32 Index = 0; !bChanged && Index < Pins.Num(); ++Index)
	{
		bChanged = Pins[Index]->PinName != OldPins[Index]->PinName ||
			!Pins[Index]->PinFriendlyName.EqualTo(OldPins[Index]->PinFriendlyName);
	}

	if (!bChanged)
	{
		for (UEdGraphPin* NewPin : Pins)
		{
			DestroyPin(NewPin);
		}
		Pins = MoveTemp(OldPins);
		return;
	}

	for (UEdGraphPin* OldPin : OldPins)
	{
		if (UEdGraphPin* NewPin = FindPin(OldPin->PinName, OldPin->Direction))
		{
			NewPin->MovePersistentDataFromOldPin(*OldPin);
		}
		else
		{
			OldPin->BreakAllPinLinks();
		}
		DestroyPin(OldPin);
	}

	MarkPackageDirty();
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

FLinearColor ULDComboEntryNode::GetNodeTitleColor() const
{
	// 시작 노드를 구분하기 위해 초록색
	return FLinearColor(0.1f, 0.6f, 0.2f);
}

void ULDComboActionNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, ComboPinCategory, ComboInputPinName);
	CreateCommandPins();
}

FText ULDComboActionNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const FText Name = ActionName.IsNone() ? LOCTEXT("NewActionTitle", "New Action") : FText::FromName(ActionName);
	if (TitleType != ENodeTitleType::FullTitle)
	{
		return Name;
	}

	// 그래프의 노드에는 두 번째 줄에 몽타주와 배율 표시
	const FText MontageName = Montage ? FText::FromString(Montage->GetName()) : LOCTEXT("NoMontage", "(No Montage)");
	return FText::Format(LOCTEXT("ActionFullTitle", "{0}\n{1}  x{2}"), Name, MontageName, FText::AsNumber(DamageMultiplier));
}

void ULDComboActionNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	// 출력 핀에서 끌어온 경우에만 새 노드의 입력에 연결
	if (FromPin && FromPin->Direction == EGPD_Output)
	{
		GetSchema()->TryCreateConnection(FromPin, FindPinChecked(ComboInputPinName));
	}
}

void ULDComboActionNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 노드 제목 갱신 + 다시 컴파일
	GetGraph()->NotifyNodeChanged(this);
}

#undef LOCTEXT_NAMESPACE
