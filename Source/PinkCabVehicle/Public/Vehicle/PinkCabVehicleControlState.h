#pragma once

#include "CoreMinimal.h"

struct FPinkCabVehicleControlState
{
    void SetSteering(float Value) { Steering = FMath::Clamp(Value, -1.0f, 1.0f); }
    void SetThrottle(float Value) { Throttle = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetBrake(float Value) { Brake = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetClutch(float Value) { Clutch = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetHandbrake(float Value) { Handbrake = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetDriveline(int32 InRequestedGear, int32 InEngagedGear, float InClutchCoupling)
    {
        RequestedGear = FMath::Clamp(InRequestedGear, -1, 5);
        EngagedGear = FMath::Clamp(InEngagedGear, -1, 5);
        ClutchCoupling = FMath::Clamp(InClutchCoupling, 0.0f, 1.0f);
    }
    void SetExternalRearDriveTorquePerWheel(float TorqueNm)
    {
        ExternalRearDriveTorquePerWheelNm = TorqueNm;
    }

    float Steering = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Clutch = 0.0f;
    float Handbrake = 0.0f;
    int32 RequestedGear = 0;
    int32 EngagedGear = 0;
    float ClutchCoupling = 1.0f;
    float ExternalRearDriveTorquePerWheelNm = 0.0f;
};
