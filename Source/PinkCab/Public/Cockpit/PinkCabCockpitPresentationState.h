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
    bool bPassengerDoorAvailable = false;
    bool bPassengerDoorOpen = false;

    bool bMeterAvailable = false;
    bool bMeterRunning = false;
    int64 FareMinor = 0;
    double MeterDistanceKm = 0.0;
    double MeterFareSeconds = 0.0;

    bool bRouteAvailable = false;
    float RouteProgress01 = 0.0f;
    bool bRadioAvailable = false;
    bool bMirrorsAvailable = false;

    bool bTurnSignalLeft = false;
    bool bTurnSignalRight = false;
    bool bHornActive = false;
    bool bLightsOn = false;
    bool bWipersOn = false;
    bool bWasherActive = false;
};
