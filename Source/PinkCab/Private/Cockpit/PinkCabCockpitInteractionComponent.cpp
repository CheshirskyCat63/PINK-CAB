#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"

UPinkCabCockpitInteractionComponent::UPinkCabCockpitInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    QuickSlots.SetNum(4);
}

FName UPinkCabCockpitInteractionComponent::TargetForQuickSlot(const int32 Slot)
{
    switch (Slot)
    {
    case 1: return TEXT("TurnSignals");
    case 2: return TEXT("Horn");
    case 3: return TEXT("Gearbox");
    case 4: return TEXT("Handbrake");
    default: return NAME_None;
    }
}

FPinkCabInteractionControlSpec UPinkCabCockpitInteractionComponent::SpecForTargetId(const FName TargetId)
{
    return PinkCabInteractionSpecForTargetId(TargetId);
}

void UPinkCabCockpitInteractionComponent::SetQuickSlotHeld(const int32 Slot, const bool bHeld)
{
    if (Slot < 1 || Slot > QuickSlots.Num()) return;
    FQuickSlotState& State = QuickSlots[Slot - 1];
    if (bHeld && !State.bHeld) State.PressSerial = NextPressSerial++;
    State.bHeld = bHeld;
}

FName UPinkCabCockpitInteractionComponent::GetCurrentQuickTargetId() const
{
    int32 BestIndex = INDEX_NONE;
    uint32 BestSerial = 0;
    for (int32 Index = 0; Index < QuickSlots.Num(); ++Index)
    {
        const FQuickSlotState& State = QuickSlots[Index];
        if (State.bHeld && State.PressSerial >= BestSerial)
        {
            BestSerial = State.PressSerial;
            BestIndex = Index;
        }
    }
    return BestIndex == INDEX_NONE ? NAME_None : TargetForQuickSlot(BestIndex + 1);
}

void UPinkCabCockpitInteractionComponent::SetCurrentTarget(const FPinkCabInteractionControlSpec& Spec)
{
    CurrentTarget = Spec;
}

FPinkCabInteractionControlSpec UPinkCabCockpitInteractionComponent::ResolveActiveSpec() const
{
    if (bMomentaryActive && !ActiveMomentaryTargetId.IsNone())
    {
        return SpecForTargetId(ActiveMomentaryTargetId);
    }
    if (bGripActive && !ActiveGripTargetId.IsNone())
    {
        return SpecForTargetId(ActiveGripTargetId);
    }
    const FName QuickTarget = GetCurrentQuickTargetId();
    return QuickTarget.IsNone() ? CurrentTarget : SpecForTargetId(QuickTarget);
}

FName UPinkCabCockpitInteractionComponent::GetCurrentTargetId() const
{
    return ResolveActiveSpec().Id;
}

bool UPinkCabCockpitInteractionComponent::BeginGrip(FPinkCabInteractionEvent& OutEvent)
{
    const FPinkCabInteractionControlSpec Spec = ResolveActiveSpec();
    if (Spec.Id.IsNone() || !Spec.bSupportsGrip) return false;
    bGripActive = true;
    ActiveGripTargetId = Spec.Id;
    OutEvent = {Spec.Id, EPinkCabInteractionGesture::GripBegin, 1};
    return true;
}

bool UPinkCabCockpitInteractionComponent::EndGrip(FPinkCabInteractionEvent& OutEvent)
{
    if (!bGripActive || ActiveGripTargetId.IsNone()) return false;
    OutEvent = {ActiveGripTargetId, EPinkCabInteractionGesture::GripBegin, -1};
    bGripActive = false;
    ActiveGripTargetId = NAME_None;
    return true;
}

bool UPinkCabCockpitInteractionComponent::BeginMomentary(const double NowSeconds, FPinkCabInteractionEvent& OutEvent)
{
    const FPinkCabInteractionControlSpec Spec = ResolveActiveSpec();
    if (Spec.Id.IsNone() || !Spec.bSupportsMomentary) return false;
    bMomentaryActive = true;
    ActiveMomentaryTargetId = Spec.Id;
    MomentaryStartSeconds = NowSeconds;
    ++ActuationSerial;
    OutEvent = {Spec.Id, EPinkCabInteractionGesture::PressHold, 1};
    return true;
}

bool UPinkCabCockpitInteractionComponent::EndMomentary(const double NowSeconds, FPinkCabInteractionEvent& OutEvent)
{
    if (!bMomentaryActive || ActiveMomentaryTargetId.IsNone()) return false;
    LastMomentaryHoldSeconds = FMath::Max(0.0, NowSeconds - MomentaryStartSeconds);
    ++ActuationSerial;
    OutEvent = {ActiveMomentaryTargetId, EPinkCabInteractionGesture::PressHold, -1};
    bMomentaryActive = false;
    ActiveMomentaryTargetId = NAME_None;
    return true;
}

bool UPinkCabCockpitInteractionComponent::BuildWheelEvent(const int32 SignedSteps, FPinkCabInteractionEvent& OutEvent)
{
    const FPinkCabInteractionControlSpec Spec = ResolveActiveSpec();
    if (SignedSteps == 0 || Spec.Id.IsNone() || !Spec.bSupportsWheel) return false;
    if (Spec.bSupportsGrip && (!bGripActive || ActiveGripTargetId != Spec.Id)) return false;
    ++ActuationSerial;
    OutEvent = {Spec.Id, EPinkCabInteractionGesture::WheelIncrement, SignedSteps};
    return true;
}

void UPinkCabCockpitInteractionComponent::ProcessFrame(
    const FPinkCabCockpitInteractionFrame& Frame,
    const UPinkCabCockpitAssemblyComponent* Assembly,
    TArray<FPinkCabInteractionEvent>& OutActuationEvents)
{
    SetGazeHeld(Frame.bGazeHeld);
    SetQuickSlotHeld(1, Frame.bQuickRecall1Held);
    SetQuickSlotHeld(2, Frame.bQuickRecall2Held);
    SetQuickSlotHeld(3, Frame.bQuickRecall3Held);
    SetQuickSlotHeld(4, Frame.bQuickRecall4Held);

    if (Frame.bGazeHeld && Assembly && !bGripActive && !bMomentaryActive)
    {
        const FName GazeTarget = Assembly->ResolveGazeTarget(
            Frame.GazeOrigin, Frame.GazeForward, Frame.GazeMaxDistanceCm, Frame.GazeCandidateBudget);
        SetCurrentTarget(SpecForTargetId(GazeTarget));
    }
    else if (!Frame.bGazeHeld && GetCurrentQuickTargetId().IsNone() && !bGripActive && !bMomentaryActive)
    {
        SetCurrentTarget({});
    }

    FPinkCabInteractionEvent Event;
    const FPinkCabInteractionControlSpec PointerSpec = ResolveActiveSpec();
    const bool bPrimaryPointerGrips = Frame.bMomentaryHeld
        && PointerSpec.bSupportsGrip && !PointerSpec.bSupportsMomentary;
    const bool bEffectiveGripHeld = Frame.bGripHeld || bPrimaryPointerGrips;
    if (bEffectiveGripHeld && !bGripActive)
    {
        BeginGrip(Event);
    }
    else if (!bEffectiveGripHeld && bGripActive)
    {
        EndGrip(Event);
    }

    if (!bPrimaryPointerGrips && Frame.bMomentaryHeld && !bMomentaryActive)
    {
        if (BeginMomentary(Frame.NowSeconds, Event))
        {
            OutActuationEvents.Add(Event);
        }
    }
    else if ((!Frame.bMomentaryHeld || bPrimaryPointerGrips) && bMomentaryActive)
    {
        if (EndMomentary(Frame.NowSeconds, Event))
        {
            OutActuationEvents.Add(Event);
        }
    }

    if (BuildWheelEvent(Frame.WheelSteps, Event))
    {
        OutActuationEvents.Add(Event);
    }
}

void UPinkCabCockpitInteractionComponent::ResetTransientInputState(
    TArray<FPinkCabInteractionEvent>* OutReleaseEvents)
{
    if (bMomentaryActive && !ActiveMomentaryTargetId.IsNone() && OutReleaseEvents)
    {
        OutReleaseEvents->Add({ActiveMomentaryTargetId, EPinkCabInteractionGesture::PressHold, -1});
        ++ActuationSerial;
    }
    bGazeHeld = false;
    for (FQuickSlotState& Slot : QuickSlots) Slot.bHeld = false;
    CurrentTarget = {};
    bGripActive = false;
    ActiveGripTargetId = NAME_None;
    bMomentaryActive = false;
    ActiveMomentaryTargetId = NAME_None;
    MomentaryStartSeconds = 0.0;
}
