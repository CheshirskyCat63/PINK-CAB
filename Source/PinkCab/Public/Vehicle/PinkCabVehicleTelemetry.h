#pragma once

#include "CoreMinimal.h"

struct FPinkCabWheelTelemetry
{
    bool bInContact = false;
    float NormalizedSuspensionLength = 1.0f;
    float SpringForce = 0.0f;
    float SlipAngle = 0.0f;
    bool bIsSlipping = false;
    float SlipMagnitude = 0.0f;
    bool bIsSkidding = false;
    float SkidMagnitude = 0.0f;
    float DriveTorque = 0.0f;
    float BrakeTorque = 0.0f;
    bool bABSActivated = false;
};

struct FPinkCabVehicleTelemetry
{
    float SpeedKmh = 0.0f;
    float EngineRpm = 0.0f;
    int32 CurrentGear = 0;
    float NormalizedSteering = 0.0f;
    float NormalizedThrottle = 0.0f;
    float NormalizedBrake = 0.0f;
    float NormalizedClutch = 0.0f;
    float NormalizedHandbrake = 0.0f;
    TArray<FPinkCabWheelTelemetry> Wheels;
};
