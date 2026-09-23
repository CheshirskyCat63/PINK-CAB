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
    float Fuel01 = 1.0f;
    float EngineTemperature01 = 0.15f;
    int32 SelectedGear = 0;
    bool bGearLeverDragging = false;
    FVector2D GearLeverCursor = FVector2D::ZeroVector;

    bool bIgnitionRunning = false;
    float Handbrake = 1.0f;
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
