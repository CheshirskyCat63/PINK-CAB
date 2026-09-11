#pragma once

#include "CoreMinimal.h"

struct FPinkCabContinuousVehicleControls
{
    void SetSteering(const float Value) { Steering = FMath::Clamp(Value, -1.0f, 1.0f); }
    void SetThrottle(const float Value) { Throttle = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetBrake(const float Value) { Brake = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetClutch(const float Value) { Clutch = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetHandbrake(const float Value) { Handbrake = FMath::Clamp(Value, 0.0f, 1.0f); }

    float GetSteering() const { return Steering; }
    float GetThrottle() const { return Throttle; }
    float GetBrake() const { return Brake; }
    float GetClutch() const { return Clutch; }
    float GetHandbrake() const { return Handbrake; }

private:
    float Steering = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Clutch = 0.0f;
    float Handbrake = 0.0f;
};
