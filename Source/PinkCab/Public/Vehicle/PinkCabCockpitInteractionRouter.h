#pragma once

#include "Interaction/PinkCabInteractionModel.h"
#include "Vehicle/PinkCabCockpitState.h"

struct FPinkCabCockpitInteractionRouter
{
    static constexpr float HandbrakeWheelStep = 1.0f / 64.0f;

    static bool Apply(const FPinkCabInteractionEvent& Event, FPinkCabCockpitState& State)
    {
        if (Event.TargetId == FName(TEXT("Gearbox")))
        {
            // Gearbox authority is RMB grip + mouse H-gate travel in the vehicle runtime.
            // Wheel input must never bypass the common engagement validator.
            return false;
        }

        if (Event.TargetId == FName(TEXT("TurnSignals")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::WheelIncrement || Event.SignedValue == 0)
            {
                return false;
            }
            State.SetTurnSignalDirection(FMath::Clamp(Event.SignedValue, -1, 1));
            return true;
        }

        if (Event.TargetId == FName(TEXT("Lights")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::WheelIncrement || Event.SignedValue == 0)
            {
                return false;
            }
            State.SetLightMode(State.GetLightMode() + Event.SignedValue);
            return true;
        }

        if (Event.TargetId == FName(TEXT("Wipers")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::WheelIncrement || Event.SignedValue == 0)
            {
                return false;
            }
            State.SetWiperMode(State.GetWiperMode() + Event.SignedValue);
            return true;
        }

        if (Event.TargetId == FName(TEXT("Horn")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::PressHold || Event.SignedValue == 0)
            {
                return false;
            }
            State.SetHornActive(Event.SignedValue > 0);
            return true;
        }

        if (Event.TargetId == FName(TEXT("Washer")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::PressHold || Event.SignedValue == 0)
            {
                return false;
            }
            State.SetWasherActive(Event.SignedValue > 0);
            return true;
        }

        if (Event.TargetId == FName(TEXT("Handbrake")))
        {
            // Handbrake analog authority is the RMB + mouse-pull actuator.
            // Wheel events must not bypass stationary latch / moving auto-return semantics.
            return false;
        }

        if (Event.TargetId == FName(TEXT("ClutchPedal")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::WheelIncrement || Event.SignedValue == 0)
            {
                return false;
            }
            State.AdjustClutchReleaseSpeed(Event.SignedValue);
            return true;
        }

        if (Event.TargetId == FName(TEXT("PassengerDoor")))
        {
            if (Event.Gesture != EPinkCabInteractionGesture::WheelIncrement || Event.SignedValue == 0)
            {
                return false;
            }
            State.SetPassengerDoorOpen(Event.SignedValue > 0);
            return true;
        }

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

        if (Event.TargetId == FName(TEXT("Meter")) || Event.TargetId == FName(TEXT("Taximeter")))
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
