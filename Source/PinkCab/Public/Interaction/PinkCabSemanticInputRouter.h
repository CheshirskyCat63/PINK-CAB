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
    PressHold,
    Wheel,
    Clutch,
    Brake,
    Throttle
};

struct FPinkCabSemanticInputRouter
{
    static FPinkCabSemanticInputRouter CreateDefaults()
    {
        FPinkCabSemanticInputRouter Router;
        Router.RestoreDefaults();
        return Router;
    }

    EPinkCabSemanticAction Resolve(const FKey& Key) const
    {
        if (const EPinkCabSemanticAction* Found = Bindings.Find(Key))
        {
            return *Found;
        }
        return EPinkCabSemanticAction::None;
    }
    bool TryRebind(EPinkCabSemanticAction Action, const FKey& NewKey)
    {
        if (Action == EPinkCabSemanticAction::None || !NewKey.IsValid())
        {
            return false;
        }
        if (const EPinkCabSemanticAction* Existing = Bindings.Find(NewKey))
        {
            if (*Existing != Action)
            {
                return false;
            }
            return true;
        }
        FKey ExistingKey;
        for (const TPair<FKey, EPinkCabSemanticAction>& Pair : Bindings)
        {
            if (Pair.Value == Action)
            {
                ExistingKey = Pair.Key;
                break;
            }
        }
        if (ExistingKey.IsValid())
        {
            Bindings.Remove(ExistingKey);
        }
        Bindings.Add(NewKey, Action);
        return true;
    }

    void RestoreDefaults()
    {
        Bindings.Reset();
        Bindings.Add(EKeys::SpaceBar, EPinkCabSemanticAction::GazeHold);
        Bindings.Add(EKeys::One, EPinkCabSemanticAction::QuickRecall1);
        Bindings.Add(EKeys::Two, EPinkCabSemanticAction::QuickRecall2);
        Bindings.Add(EKeys::Three, EPinkCabSemanticAction::QuickRecall3);
        Bindings.Add(EKeys::Four, EPinkCabSemanticAction::QuickRecall4);
        Bindings.Add(EKeys::RightMouseButton, EPinkCabSemanticAction::Grip);
        Bindings.Add(EKeys::LeftMouseButton, EPinkCabSemanticAction::PressHold);
        Bindings.Add(EKeys::MouseWheelAxis, EPinkCabSemanticAction::Wheel);
        Bindings.Add(EKeys::Q, EPinkCabSemanticAction::Clutch);
        Bindings.Add(EKeys::W, EPinkCabSemanticAction::Brake);
        Bindings.Add(EKeys::E, EPinkCabSemanticAction::Throttle);
    }

private:
    TMap<FKey, EPinkCabSemanticAction> Bindings;
};
