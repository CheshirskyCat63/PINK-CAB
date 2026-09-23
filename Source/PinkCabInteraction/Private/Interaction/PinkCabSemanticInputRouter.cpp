#include "Interaction/PinkCabSemanticInputRouter.h"

FPinkCabSemanticInputRouter
FPinkCabSemanticInputRouter::CreateDefaults()
{
    FPinkCabSemanticInputRouter Router;
    Router.RestoreDefaults();
    return Router;
}

EPinkCabSemanticAction FPinkCabSemanticInputRouter::Resolve(
    const FKey& Key) const
{
    if (const EPinkCabSemanticAction* Found = Bindings.Find(Key))
    {
        return *Found;
    }
    return EPinkCabSemanticAction::None;
}

FKey FPinkCabSemanticInputRouter::GetKeyForAction(
    EPinkCabSemanticAction Action) const
{
    for (const TPair<FKey, EPinkCabSemanticAction>& Pair : Bindings)
    {
        if (Pair.Value == Action)
        {
            return Pair.Key;
        }
    }
    return FKey();
}
bool FPinkCabSemanticInputRouter::TryRebind(
    EPinkCabSemanticAction Action,
    const FKey& NewKey)
{
    if (Action == EPinkCabSemanticAction::None || !NewKey.IsValid())
    {
        return false;
    }

    if (const EPinkCabSemanticAction* Existing =
        Bindings.Find(NewKey))
    {
        return *Existing == Action;
    }

    const FKey ExistingKey = GetKeyForAction(Action);
    if (ExistingKey.IsValid())
    {
        Bindings.Remove(ExistingKey);
    }
    Bindings.Add(NewKey, Action);
    return true;
}

void FPinkCabSemanticInputRouter::RestoreDefaults()
{
    Bindings.Reset();
    Bindings.Add(
        EKeys::SpaceBar,
        EPinkCabSemanticAction::GazeHold);
    Bindings.Add(
        EKeys::One,
        EPinkCabSemanticAction::QuickRecall1);
    Bindings.Add(
        EKeys::Two,
        EPinkCabSemanticAction::QuickRecall2);
    Bindings.Add(
        EKeys::Three,
        EPinkCabSemanticAction::QuickRecall3);
    Bindings.Add(
        EKeys::Four,
        EPinkCabSemanticAction::QuickRecall4);
    Bindings.Add(
        EKeys::RightMouseButton,
        EPinkCabSemanticAction::Grip);
    Bindings.Add(
        EKeys::LeftMouseButton,
        EPinkCabSemanticAction::MomentaryPress);
    Bindings.Add(
        EKeys::MouseWheelAxis,
        EPinkCabSemanticAction::Wheel);
    Bindings.Add(
        EKeys::Q,
        EPinkCabSemanticAction::Clutch);
    Bindings.Add(
        EKeys::W,
        EPinkCabSemanticAction::Brake);
    Bindings.Add(
        EKeys::E,
        EPinkCabSemanticAction::Throttle);
}
