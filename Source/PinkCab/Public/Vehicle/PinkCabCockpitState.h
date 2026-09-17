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
    static constexpr float ClutchReleaseMinSeconds = 0.20f;
    static constexpr float ClutchReleaseMaxSeconds = 1.20f;
    static constexpr int32 ClutchReleaseIntervals = 64;
    static constexpr float ClutchReleaseStepSeconds =
        (ClutchReleaseMaxSeconds - ClutchReleaseMinSeconds) / static_cast<float>(ClutchReleaseIntervals);

    EPinkCabIgnitionState GetIgnitionState() const { return IgnitionState; }
    int32 GetSelectedGear() const { return SelectedGear; }
    float GetClutchReleaseSeconds() const { return ClutchReleaseSeconds; }
    float GetHandbrakeAmount() const { return HandbrakeAmount; }
    bool IsHandbrakeEngaged() const { return HandbrakeAmount > KINDA_SMALL_NUMBER; }
    bool IsPassengerDoorOpen() const { return bPassengerDoorOpen; }
    EPinkCabMeterState GetMeterState() const { return MeterState; }
    int32 GetTurnSignalDirection() const { return TurnSignalDirection; }
    bool IsHornActive() const { return bHornActive; }
    int32 GetLightMode() const { return LightMode; }
    int32 GetWiperMode() const { return WiperMode; }
    bool IsWasherActive() const { return bWasherActive; }

    bool StartEngine()
    {
        if (IgnitionState == EPinkCabIgnitionState::Running)
        {
            return false;
        }
        IgnitionState = EPinkCabIgnitionState::Running;
        return true;
    }

    void StopEngine() { IgnitionState = EPinkCabIgnitionState::Off; }

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

    float AdjustClutchReleaseSpeed(int32 SignedSteps)
    {
        ClutchReleaseSeconds = FMath::Clamp(
            ClutchReleaseSeconds + static_cast<float>(SignedSteps) * ClutchReleaseStepSeconds,
            ClutchReleaseMinSeconds,
            ClutchReleaseMaxSeconds);
        return ClutchReleaseSeconds;
    }

    void SetHandbrakeAmount(float Amount) { HandbrakeAmount = FMath::Clamp(Amount, 0.0f, 1.0f); }
    void SetHandbrakeEngaged(bool bEngaged) { SetHandbrakeAmount(bEngaged ? 1.0f : 0.0f); }
    void SetPassengerDoorOpen(bool bOpen) { bPassengerDoorOpen = bOpen; }
    void SetTurnSignalDirection(int32 Direction) { TurnSignalDirection = FMath::Clamp(Direction, -1, 1); }
    void SetHornActive(bool bActive) { bHornActive = bActive; }
    void SetLightMode(int32 Mode) { LightMode = FMath::Clamp(Mode, 0, 2); }
    void SetWiperMode(int32 Mode) { WiperMode = FMath::Clamp(Mode, 0, 2); }
    void SetWasherActive(bool bActive) { bWasherActive = bActive; }

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

    void ResetMeter() { MeterState = EPinkCabMeterState::Off; }

private:
    EPinkCabIgnitionState IgnitionState = EPinkCabIgnitionState::Off;
    int32 SelectedGear = 0;
    float ClutchReleaseSeconds = 0.70f;
    float HandbrakeAmount = 1.0f;
    bool bPassengerDoorOpen = false;
    EPinkCabMeterState MeterState = EPinkCabMeterState::Off;
    int32 TurnSignalDirection = 0;
    bool bHornActive = false;
    int32 LightMode = 0;
    int32 WiperMode = 0;
    bool bWasherActive = false;
};
