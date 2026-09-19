#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"

enum class EPinkCabSemanticAction : uint8
{
    None,
    GazeHold,
    QuickRecall1,
    QuickRecall2,
    QuickRecall3,
    QuickRecall4,
    Grip,
    MomentaryPress,
    Wheel,
    Clutch,
    Brake,
    Throttle
};

struct PINKCABINTERACTION_API FPinkCabSemanticInputRouter
{
    static FPinkCabSemanticInputRouter CreateDefaults();

    EPinkCabSemanticAction Resolve(const FKey& Key) const;
    FKey GetKeyForAction(EPinkCabSemanticAction Action) const;
    bool TryRebind(
        EPinkCabSemanticAction Action,
        const FKey& NewKey);
    void RestoreDefaults();

private:
    TMap<FKey, EPinkCabSemanticAction> Bindings;
};
