#include "Vehicle/PinkCabCockpitState.h"

EPinkCabIgnitionState FPinkCabCockpitState::GetIgnitionState() const { return IgnitionState; }
int32 FPinkCabCockpitState::GetSelectedGear() const { return SelectedGear; }
float FPinkCabCockpitState::GetClutchReleaseSeconds() const { return ClutchReleaseSeconds; }
float FPinkCabCockpitState::GetHandbrakeAmount() const { return HandbrakeAmount; }
bool FPinkCabCockpitState::IsHandbrakeEngaged() const { return HandbrakeAmount > KINDA_SMALL_NUMBER; }
bool FPinkCabCockpitState::IsPassengerDoorOpen() const { return bPassengerDoorOpen; }
EPinkCabMeterState FPinkCabCockpitState::GetMeterState() const { return MeterState; }
int32 FPinkCabCockpitState::GetTurnSignalDirection() const { return TurnSignalDirection; }
bool FPinkCabCockpitState::IsHornActive() const { return bHornActive; }
int32 FPinkCabCockpitState::GetLightMode() const { return LightMode; }
int32 FPinkCabCockpitState::GetWiperMode() const { return WiperMode; }
bool FPinkCabCockpitState::IsWasherActive() const { return bWasherActive; }

bool FPinkCabCockpitState::StartEngine()
{
    if (IgnitionState == EPinkCabIgnitionState::Running) return false;
    IgnitionState = EPinkCabIgnitionState::Running;
    return true;
}

void FPinkCabCockpitState::StopEngine() { IgnitionState = EPinkCabIgnitionState::Off; }

void FPinkCabCockpitState::StallEngine()
{
    if (IgnitionState == EPinkCabIgnitionState::Running)
    {
        IgnitionState = EPinkCabIgnitionState::Stalled;
    }
}

int32 FPinkCabCockpitState::ShiftBy(int32 Delta)
{
    SelectedGear = FMath::Clamp(SelectedGear + Delta, -1, 5);
    return SelectedGear;
}

void FPinkCabCockpitState::SetSelectedGear(int32 Gear) { SelectedGear = FMath::Clamp(Gear, -1, 5); }

float FPinkCabCockpitState::AdjustClutchReleaseSpeed(int32 SignedSteps)
{
    ClutchReleaseSeconds = FMath::Clamp(
        ClutchReleaseSeconds + static_cast<float>(SignedSteps) * ClutchReleaseStepSeconds,
        ClutchReleaseMinSeconds,
        ClutchReleaseMaxSeconds);
    return ClutchReleaseSeconds;
}

void FPinkCabCockpitState::SetHandbrakeAmount(float Amount) { HandbrakeAmount = FMath::Clamp(Amount, 0.0f, 1.0f); }
void FPinkCabCockpitState::SetHandbrakeEngaged(bool bEngaged) { SetHandbrakeAmount(bEngaged ? 1.0f : 0.0f); }
void FPinkCabCockpitState::SetPassengerDoorOpen(bool bOpen) { bPassengerDoorOpen = bOpen; }
void FPinkCabCockpitState::SetTurnSignalDirection(int32 Direction) { TurnSignalDirection = FMath::Clamp(Direction, -1, 1); }
void FPinkCabCockpitState::SetHornActive(bool bActive) { bHornActive = bActive; }
void FPinkCabCockpitState::SetLightMode(int32 Mode) { LightMode = FMath::Clamp(Mode, 0, 2); }
void FPinkCabCockpitState::SetWiperMode(int32 Mode) { WiperMode = FMath::Clamp(Mode, 0, 2); }
void FPinkCabCockpitState::SetWasherActive(bool bActive) { bWasherActive = bActive; }

bool FPinkCabCockpitState::StartMeter()
{
    if (MeterState == EPinkCabMeterState::Running) return false;
    MeterState = EPinkCabMeterState::Running;
    return true;
}

bool FPinkCabCockpitState::StopMeter()
{
    if (MeterState != EPinkCabMeterState::Running) return false;
    MeterState = EPinkCabMeterState::Stopped;
    return true;
}

void FPinkCabCockpitState::ResetMeter() { MeterState = EPinkCabMeterState::Off; }
