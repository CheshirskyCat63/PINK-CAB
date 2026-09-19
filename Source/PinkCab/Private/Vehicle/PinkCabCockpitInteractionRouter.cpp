#include "Vehicle/PinkCabCockpitInteractionRouter.h"

namespace
{
bool IsWheelIncrement(const FPinkCabInteractionEvent& Event)
{
    return Event.Gesture == EPinkCabInteractionGesture::WheelIncrement
        && Event.SignedValue != 0;
}

bool ApplyWheelControl(const FPinkCabInteractionEvent& Event, FPinkCabCockpitState& State)
{
    if (!IsWheelIncrement(Event))
    {
        return false;
    }
    if (Event.TargetId == FName(TEXT("TurnSignals")))
    {
        State.SetTurnSignalDirection(FMath::Clamp(Event.SignedValue, -1, 1));
        return true;
    }
    if (Event.TargetId == FName(TEXT("Lights")))
    {
        State.SetLightMode(State.GetLightMode() + Event.SignedValue);
        return true;
    }
    if (Event.TargetId == FName(TEXT("Wipers")))
    {
        State.SetWiperMode(State.GetWiperMode() + Event.SignedValue);
        return true;
    }
    if (Event.TargetId == FName(TEXT("ClutchPedal")))
    {
        State.AdjustClutchReleaseSpeed(Event.SignedValue);
        return true;
    }
    if (Event.TargetId == FName(TEXT("PassengerDoor")))
    {
        State.SetPassengerDoorOpen(Event.SignedValue > 0);
        return true;
    }
    return false;
}

bool ApplyMomentaryControl(const FPinkCabInteractionEvent& Event, FPinkCabCockpitState& State)
{
    if (Event.Gesture != EPinkCabInteractionGesture::PressHold || Event.SignedValue == 0)
    {
        return false;
    }
    if (Event.TargetId == FName(TEXT("Horn")))
    {
        State.SetHornActive(Event.SignedValue > 0);
        return true;
    }
    if (Event.TargetId == FName(TEXT("Washer")))
    {
        State.SetWasherActive(Event.SignedValue > 0);
        return true;
    }
    return false;
}

bool ApplyToggleControl(const FPinkCabInteractionEvent& Event, FPinkCabCockpitState& State)
{
    if (Event.Gesture != EPinkCabInteractionGesture::PressHold || Event.SignedValue <= 0)
    {
        return false;
    }
    if (Event.TargetId == FName(TEXT("Ignition")))
    {
        if (State.GetIgnitionState() == EPinkCabIgnitionState::Running)
        {
            State.StopEngine();
        }
        else
        {
            State.StartEngine();
        }
        return true;
    }
    if (Event.TargetId != FName(TEXT("Meter")) && Event.TargetId != FName(TEXT("Taximeter")))
    {
        return false;
    }
    if (State.GetMeterState() == EPinkCabMeterState::Off)
    {
        State.StartMeter();
    }
    else if (State.GetMeterState() == EPinkCabMeterState::Running)
    {
        State.StopMeter();
    }
    else
    {
        State.ResetMeter();
    }
    return true;
}
}

bool FPinkCabCockpitInteractionRouter::Apply(
    const FPinkCabInteractionEvent& Event,
    FPinkCabCockpitState& State)
{
    // H-gate and handbrake analog authority stay in the vehicle runtime.
    if (Event.TargetId == FName(TEXT("Gearbox"))
        || Event.TargetId == FName(TEXT("Handbrake")))
    {
        return false;
    }
    if (ApplyWheelControl(Event, State))
    {
        return true;
    }
    if (ApplyMomentaryControl(Event, State))
    {
        return true;
    }
    return ApplyToggleControl(Event, State);
}

