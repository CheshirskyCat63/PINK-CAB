#pragma once

#include "CoreMinimal.h"

struct FPinkCabVehicleControlState
{
    void SetSteering(float Value) { Steering = FMath::Clamp(Value, -1.0f, 1.0f); }
    void SetThrottle(float Value) { Throttle = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetBrake(float Value) { Brake = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetClutch(float Value) { Clutch = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetHandbrake(float Value) { Handbrake = FMath::Clamp(Value, 0.0f, 1.0f); }

    float Steering = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Clutch = 0.0f;
    float Handbrake = 0.0f;
};
