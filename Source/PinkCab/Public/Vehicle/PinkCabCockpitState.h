#pragma once

#include "CoreMinimal.h"

enum class EPinkCabIgnitionState : uint8
{
    Off,
    Running,
    Stalled
};

enum class EPinkCabMeterState : uint8
{
    Off,
    Running,
    Stopped
};

struct FPinkCabCockpitState
{
    EPinkCabIgnitionState GetIgnitionState() const { return IgnitionState; }
    int32 GetSelectedGear() const { return SelectedGear; }
    bool IsHandbrakeEngaged() const { return bHandbrakeEngaged; }
    bool IsPassengerDoorOpen() const { return bPassengerDoorOpen; }
    EPinkCabMeterState GetMeterState() const { return MeterState; }

    bool StartEngine()
    {
        if (IgnitionState == EPinkCabIgnitionState::Running)
        {
            return false;
        }
        IgnitionState = EPinkCabIgnitionState::Running;
        return true;
    }

    void StopEngine()
    {
        IgnitionState = EPinkCabIgnitionState::Off;
    }

    void StallEngine()
    {
        if (IgnitionState == EPinkCabIgnitionState::Running)
        {
            IgnitionState = EPinkCabIgnitionState::Stalled;
        }
    }

    int32 ShiftBy(int32 Delta)
    {
        SelectedGear = FMath::Clamp(SelectedGear + Delta, -1, 5);
        return SelectedGear;
    }

    void SetHandbrakeEngaged(bool bEngaged)
    {
        bHandbrakeEngaged = bEngaged;
    }

    void SetPassengerDoorOpen(bool bOpen)
    {
        bPassengerDoorOpen = bOpen;
    }
    bool StartMeter()
    {
        if (MeterState == EPinkCabMeterState::Running)
        {
            return false;
        }
        MeterState = EPinkCabMeterState::Running;
        return true;
    }

    bool StopMeter()
    {
        if (MeterState != EPinkCabMeterState::Running)
        {
            return false;
        }
        MeterState = EPinkCabMeterState::Stopped;
        return true;
    }

    void ResetMeter()
    {
        MeterState = EPinkCabMeterState::Off;
    }

private:
    EPinkCabIgnitionState IgnitionState = EPinkCabIgnitionState::Off;
    int32 SelectedGear = 0;
    bool bHandbrakeEngaged = true;
    bool bPassengerDoorOpen = false;
    EPinkCabMeterState MeterState = EPinkCabMeterState::Off;
};
