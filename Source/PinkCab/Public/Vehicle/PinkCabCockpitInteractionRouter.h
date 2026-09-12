#pragma once

#include "Interaction/PinkCabInteractionModel.h"
#include "Vehicle/PinkCabCockpitState.h"

struct FPinkCabCockpitInteractionRouter
{
    static bool Apply(const FPinkCabInteractionEvent& Event, FPinkCabCockpitState& State)
    {
        if (Event.TargetId == FName(TEXT("Gearbox")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::WheelIncrement || Event.SignedValue == 0)
            {
                return false;
            }
            State.ShiftBy(Event.SignedValue);
            return true;
        }

        if (Event.Gesture != EPinkCabInteractionGesture::PressHold || Event.SignedValue <= 0)
        {
            return false;
        }

        if (Event.TargetId == FName(TEXT("Handbrake")))
        {
            State.SetHandbrakeEngaged(!State.IsHandbrakeEngaged());
            return true;
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

        if (Event.TargetId == FName(TEXT("PassengerDoor")))
        {
            State.SetPassengerDoorOpen(!State.IsPassengerDoorOpen());
            return true;
        }

        if (Event.TargetId == FName(TEXT("Meter")))
        {
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

        return false;
    }
};
