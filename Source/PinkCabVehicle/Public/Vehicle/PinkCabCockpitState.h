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

struct PINKCABVEHICLE_API FPinkCabCockpitState
{
    static constexpr float ClutchReleaseMinSeconds = 0.20f;
    static constexpr float ClutchReleaseMaxSeconds = 1.20f;
    static constexpr int32 ClutchReleaseIntervals = 64;
    static constexpr float ClutchReleaseStepSeconds =
        (ClutchReleaseMaxSeconds - ClutchReleaseMinSeconds) / static_cast<float>(ClutchReleaseIntervals);

    EPinkCabIgnitionState GetIgnitionState() const;
    int32 GetSelectedGear() const;
    float GetClutchReleaseSeconds() const;
    float GetHandbrakeAmount() const;
    bool IsHandbrakeEngaged() const;
    bool IsPassengerDoorOpen() const;
    EPinkCabMeterState GetMeterState() const;
    int32 GetTurnSignalDirection() const;
    bool IsHornActive() const;
    int32 GetLightMode() const;
    int32 GetWiperMode() const;
    bool IsWasherActive() const;

    bool StartEngine();
    void StopEngine();
    void StallEngine();
    int32 ShiftBy(int32 Delta);
    void SetSelectedGear(int32 Gear);
    float AdjustClutchReleaseSpeed(int32 SignedSteps);
    void SetHandbrakeAmount(float Amount);
    void SetHandbrakeEngaged(bool bEngaged);
    void SetPassengerDoorOpen(bool bOpen);
    void SetTurnSignalDirection(int32 Direction);
    void SetHornActive(bool bActive);
    void SetLightMode(int32 Mode);
    void SetWiperMode(int32 Mode);
    void SetWasherActive(bool bActive);
    bool StartMeter();
    bool StopMeter();
    void ResetMeter();

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
