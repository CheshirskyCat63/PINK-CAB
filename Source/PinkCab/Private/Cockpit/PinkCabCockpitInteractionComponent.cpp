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

bool UPinkCabCockpitInteractionComponent::IsLeverManipulationTarget(const FName TargetId)
{
    return TargetId == TEXT("Gearbox") || TargetId == TEXT("Handbrake");
}

FPinkCabInteractionControlSpec UPinkCabCockpitInteractionComponent::SpecForTargetId(const FName TargetId)
{
    return PinkCabInteractionSpecForTargetId(TargetId);
}

void UPinkCabCockpitInteractionComponent::SetQuickSlotHeld(const int32 Slot, const bool bHeld)
{
    if (Slot < 1 || Slot > QuickSlots.Num()) return;
    FQuickSlotState& State = QuickSlots[Slot - 1];
    if (bHeld && !State.bHeld)
    {
        State.PressSerial = NextPressSerial++;
        CurrentTarget = SpecForTargetId(TargetForQuickSlot(Slot));
        bCurrentTargetFromQuickRecall = !CurrentTarget.Id.IsNone();
        bCurrentTargetRecallConsumed = false;
    }
    State.bHeld = bHeld;
    ClearConsumedRecallIfIdle();
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
    bCurrentTargetFromQuickRecall = false;
    bCurrentTargetRecallConsumed = false;
}

void UPinkCabCockpitInteractionComponent::MarkRecalledTargetConsumed(const FName TargetId)
{
    if (TargetId.IsNone()) return;
    if (bCurrentTargetFromQuickRecall && CurrentTarget.Id == TargetId)
    {
        bCurrentTargetRecallConsumed = true;
    }
}

void UPinkCabCockpitInteractionComponent::ClearConsumedRecallIfIdle()
{
    if (!bCurrentTargetFromQuickRecall
        || !bCurrentTargetRecallConsumed
        || bGripActive
        || bMomentaryActive
        || !GetCurrentQuickTargetId().IsNone())
    {
        return;
    }

    CurrentTarget = {};
    bCurrentTargetFromQuickRecall = false;
    bCurrentTargetRecallConsumed = false;
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
    if (!QuickTarget.IsNone())
    {
        return SpecForTargetId(QuickTarget);
    }
    if (bGearboxStageFromClutchHeld)
    {
        return SpecForTargetId(TEXT("Gearbox"));
    }
    return CurrentTarget;
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
    MarkRecalledTargetConsumed(Spec.Id);
    OutEvent = {Spec.Id, EPinkCabInteractionGesture::GripBegin, 1};
    return true;
}

bool UPinkCabCockpitInteractionComponent::EndGrip(FPinkCabInteractionEvent& OutEvent)
{
    if (!bGripActive || ActiveGripTargetId.IsNone()) return false;
    OutEvent = {ActiveGripTargetId, EPinkCabInteractionGesture::GripBegin, -1};
    bGripActive = false;
    bManipulationActive = false;
    ActiveGripTargetId = NAME_None;
    ClearConsumedRecallIfIdle();
    return true;
}

bool UPinkCabCockpitInteractionComponent::BeginMomentary(const double NowSeconds, FPinkCabInteractionEvent& OutEvent)
{
    const FPinkCabInteractionControlSpec Spec = ResolveActiveSpec();
    if (Spec.Id.IsNone() || !Spec.bSupportsMomentary) return false;
    bMomentaryActive = true;
    ActiveMomentaryTargetId = Spec.Id;
    MarkRecalledTargetConsumed(Spec.Id);
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
    ClearConsumedRecallIfIdle();
    return true;
}

bool UPinkCabCockpitInteractionComponent::BuildWheelEvent(const int32 SignedSteps, FPinkCabInteractionEvent& OutEvent)
{
    const FPinkCabInteractionControlSpec Spec = ResolveActiveSpec();
    if (SignedSteps == 0 || Spec.Id.IsNone() || !Spec.bSupportsWheel) return false;
    if (Spec.bSupportsGrip && (!bGripActive || ActiveGripTargetId != Spec.Id)) return false;
    ++ActuationSerial;
    MarkRecalledTargetConsumed(Spec.Id);
    OutEvent = {Spec.Id, EPinkCabInteractionGesture::WheelIncrement, SignedSteps};
    ClearConsumedRecallIfIdle();
    return true;
}

void UPinkCabCockpitInteractionComponent::UpdateTargetSelection(
    const FPinkCabCockpitInteractionFrame& Frame,
    const UPinkCabCockpitAssemblyComponent* Assembly)
{
    if (bGripActive || bMomentaryActive)
    {
        return;
    }

    // A recalled control (1/2/3/4, including Q staging the gearbox) is an
    // explicit driver choice. RMB must grip that target instead of replacing
    // it with a center-ray miss on the next frame.
    if (Frame.bGripHeld
        && ((bCurrentTargetFromQuickRecall && !CurrentTarget.Id.IsNone())
            || bGearboxStageFromClutchHeld))
    {
        return;
    }

    // Space is an explicit gaze-selection mode and may intentionally replace
    // the staged target with what the driver is looking at.
    if (Frame.bGazeHeld && Assembly)
    {
        const FName GazeTarget = Assembly->ResolveGazeTarget(
            Frame.GazeOrigin, Frame.GazeForward, Frame.GazeMaxDistanceCm, Frame.GazeCandidateBudget);
        SetCurrentTarget(SpecForTargetId(GazeTarget));
        return;
    }

    // RMB in ordinary driving mode performs a bounded contextual center pick
    // only when there is no explicit quick-recall target waiting to be used.
    if (Frame.bGripHeld && Assembly && !bCurrentTargetFromQuickRecall)
    {
        const FName ContextTarget = Assembly->ResolveGazeTarget(
            Frame.GazeOrigin, Frame.GazeForward, Frame.GazeMaxDistanceCm, Frame.GazeCandidateBudget);
        SetCurrentTarget(SpecForTargetId(ContextTarget));
        return;
    }

    if (!Frame.bGazeHeld
        && GetCurrentQuickTargetId().IsNone()
        && !bCurrentTargetFromQuickRecall)
    {
        SetCurrentTarget({});
    }
}

void UPinkCabCockpitInteractionComponent::UpdateGripState(
    const FPinkCabCockpitInteractionFrame& Frame,
    TArray<FPinkCabInteractionEvent>& OutActuationEvents)
{
    FPinkCabInteractionEvent Event;
    if (Frame.bGripHeld && !bGripActive)
    {
        if (BeginGrip(Event)) OutActuationEvents.Add(Event);
    }
    else if (!Frame.bGripHeld && bGripActive)
    {
        if (EndGrip(Event)) OutActuationEvents.Add(Event);
    }
}

void UPinkCabCockpitInteractionComponent::UpdateManipulationState(
    const FPinkCabCockpitInteractionFrame& Frame)
{
    bManipulationActive =
        bGripActive
        && IsLeverManipulationTarget(ActiveGripTargetId)
        && Frame.bMomentaryHeld;
}

void UPinkCabCockpitInteractionComponent::UpdateMomentaryState(
    const FPinkCabCockpitInteractionFrame& Frame,
    TArray<FPinkCabInteractionEvent>& OutActuationEvents)
{
    FPinkCabInteractionEvent Event;
    if (Frame.bMomentaryHeld && !bMomentaryActive)
    {
        if (BeginMomentary(Frame.NowSeconds, Event)) OutActuationEvents.Add(Event);
        return;
    }
    if (!Frame.bMomentaryHeld && bMomentaryActive
        && EndMomentary(Frame.NowSeconds, Event))
    {
        OutActuationEvents.Add(Event);
    }
}

void UPinkCabCockpitInteractionComponent::AppendWheelEvent(
    const FPinkCabCockpitInteractionFrame& Frame,
    TArray<FPinkCabInteractionEvent>& OutActuationEvents)
{
    FPinkCabInteractionEvent Event;
    if (BuildWheelEvent(Frame.WheelSteps, Event)) OutActuationEvents.Add(Event);
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
    bGearboxStageFromClutchHeld = Frame.bGearboxStageFromClutchHeld;

    UpdateTargetSelection(Frame, Assembly);
    UpdateGripState(Frame, OutActuationEvents);
    UpdateManipulationState(Frame);
    UpdateMomentaryState(Frame, OutActuationEvents);
    AppendWheelEvent(Frame, OutActuationEvents);
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
    bCurrentTargetFromQuickRecall = false;
    bCurrentTargetRecallConsumed = false;
    bGearboxStageFromClutchHeld = false;
    bGripActive = false;
    bManipulationActive = false;
    ActiveGripTargetId = NAME_None;
    bMomentaryActive = false;
    ActiveMomentaryTargetId = NAME_None;
    MomentaryStartSeconds = 0.0;
}
