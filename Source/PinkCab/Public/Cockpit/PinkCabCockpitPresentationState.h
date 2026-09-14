#pragma once

#include "CoreMinimal.h"

struct FPinkCabCockpitPresentationState
{
    float Steering = 0.0f;
    float Clutch = 0.0f;
    float Brake = 0.0f;
    float Throttle = 0.0f;
    float SpeedKmh = 0.0f;
    float EngineRpm = 0.0f;
    int32 SelectedGear = 0;

    bool bIgnitionRunning = false;
    bool bHandbrakeEngaged = true;
    bool bPassengerDoorOpen = false;
    bool bMeterAvailable = false;
    bool bMeterRunning = false;
    bool bTurnSignalLeft = false;
    bool bTurnSignalRight = false;
    bool bHornActive = false;
    bool bLightsOn = false;
    bool bWipersOn = false;
    bool bWasherActive = false;
};
