// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LDComboDataAsset.h"
#include "Animation/AnimMontage.h"
#include "Misc/DataValidation.h"
#include "Log/LDLog.h"
#include "Types/LDCombatTypes.h"


namespace
{
    FString CommandToString(ELDAttackCommand Command)
    {
        return UEnum::GetDisplayValueAsText(Command).ToString();
    }
}

int32 ULDComboDataAsset::FindActionIndex(FName ActionName) const
{
    return Actions.IndexOfByPredicate([ActionName](const FLDComboAction& Action)
        {
            return Action.ActionName == ActionName;
        });
}

#if WITH_EDITOR
EDataValidationResult ULDComboDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    Super::IsDataValid(Context);

    // 1. 액션 자체 검사
    TSet<FName> ActionNames;
    for (int32 i = 0; i < Actions.Num(); ++i)
    {
        const FLDComboAction& Action = Actions[i];

        if (Action.ActionName.IsNone())
        {
            Context.AddError(FText::FromString(FString::Printf(TEXT("Actions[%d]: ActionName이 비어 있습니다."), i)));
        }
        else if (ActionNames.Contains(Action.ActionName))
        {
            Context.AddError(FText::FromString(FString::Printf(TEXT("Actions[%d]: ActionName '%s'이 중복됩니다."), i, *Action.ActionName.ToString())));
        }
        ActionNames.Add(Action.ActionName);

        if (!Action.Montage)
        {
            Context.AddError(FText::FromString(FString::Printf(TEXT("'%s': Montage가 비어 있습니다."), *Action.ActionName.ToString())));
        }
    }

    // 2. 전이 검사 (Start + 각 액션의 NextActions 공통)
    auto ValidateNexts = [&](const TArray<FLDComboNext>& Nexts, const FString& OwnerName)
        {
            TSet<ELDAttackCommand> UsedCommands;
            for (const FLDComboNext& Next : Nexts)
            {
                if (UsedCommands.Contains(Next.Command))
                {
                    Context.AddError(FText::FromString(FString::Printf(TEXT("'%s': 커맨드 '%s'가 중복됩니다."), *OwnerName, *CommandToString(Next.Command))));
                }
                UsedCommands.Add(Next.Command);

                if (!ActionNames.Contains(Next.ToAction))
                {
                    Context.AddError(FText::FromString(FString::Printf(TEXT("'%s': 존재하지 않는 액션 '%s'로 전이합니다."), *OwnerName, *Next.ToAction.ToString())));
                }
            }
        };

    ValidateNexts(StartActions, TEXT("Start"));
    for (const FLDComboAction& Action : Actions)
    {
        ValidateNexts(Action.NextActions, Action.ActionName.ToString());
    }

    // 3. Start에서 도달할 수 없는 액션 검사 (경고)
    TSet<FName> Reached;
    TArray<FName> Stack;
    for (const FLDComboNext& Next : StartActions)
    {
        Stack.Push(Next.ToAction);
    }

    while (Stack.Num() > 0)
    {
        const FName Current = Stack.Pop();
        if (Reached.Contains(Current))
        {
            continue;
        }
        Reached.Add(Current);

        const int32 Index = FindActionIndex(Current);
        if (Index == INDEX_NONE)
        {
            continue;
        }

        for (const FLDComboNext& Next : Actions[Index].NextActions)
        {
            Stack.Push(Next.ToAction);
        }
    }

    for (const FLDComboAction& Action : Actions)
    {
        if (!Reached.Contains(Action.ActionName))
        {
            Context.AddWarning(FText::FromString(FString::Printf(TEXT("'%s': Start에서 도달할 수 없는 액션입니다."), *Action.ActionName.ToString())));
        }
    }

    return Context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

void ULDComboDataAsset::PrintComboTree()
{
    FString Out = TEXT("\n[Start]\n");
    TSet<FName> Path;

    for (int32 i = 0; i < StartActions.Num(); ++i)
    {
        AppendTree(StartActions[i], TEXT(""), i == StartActions.Num() - 1, Path, Out);
    }

    UE_LOG(LDLog, Log, TEXT("%s"), *Out);
}

TArray<FName> ULDComboDataAsset::GetActionNameOptions() const
{
	TArray<FName> Options;

    for (const FLDComboAction& Action : Actions)
    {
        if (!Action.ActionName.IsNone())
        {
			Options.Add(Action.ActionName);
        }
	}

    return Options;
}

void ULDComboDataAsset::AppendTree(const FLDComboNext& Next, const FString& Prefix, bool bIsLast, TSet<FName>& Path, FString& Out) const
{
    Out += Prefix + (bIsLast ? TEXT("└─ ") : TEXT("├─ "));
    Out += CommandToString(Next.Command) + TEXT(" → ") + Next.ToAction.ToString();

    // 현재 경로에 이미 있는 액션 = 순환
    if (Path.Contains(Next.ToAction))
    {
        Out += TEXT(" (loop)\n");
        return;
    }

    const int32 Index = FindActionIndex(Next.ToAction);
    if (Index == INDEX_NONE)
    {
        Out += TEXT(" (없는 액션!)\n");
        return;
    }
    Out += TEXT("\n");

    Path.Add(Next.ToAction);

    const FString ChildPrefix = Prefix + (bIsLast ? TEXT("    ") : TEXT("│   "));
    const TArray<FLDComboNext>& Children = Actions[Index].NextActions;
    for (int32 i = 0; i < Children.Num(); ++i)
    {
        AppendTree(Children[i], ChildPrefix, i == Children.Num() - 1, Path, Out);
    }

    Path.Remove(Next.ToAction);
}