#pragma once

#include "CoreMinimal.h"
#include "LDBossPattern.generated.h"

UENUM(BlueprintType)
enum class EBossPattern : uint8 {
    None UMETA(Hidden),
    NormalAttack,
    CounterStance,
};