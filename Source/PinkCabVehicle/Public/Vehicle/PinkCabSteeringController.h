#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

struct FPinkCabSteeringControllerConfig
{
    float MouseCountsForFullScale = 1400.0f;
    float CenterExponent = 1.10f;
    float StationaryResponsePerSecond = 6.0f;
    float MovingResponseLowPerSecond = 16.0f;
    float MovingResponseHighPerSecond = 22.0f;
    float HighSpeedKmh = 160.0f;
    float HighSpeedTargetGain = 1.12f;
};

class PINKCABVEHICLE_API FPinkCabSteeringController
{
public:
    explicit FPinkCabSteeringController(const FPinkCabSteeringControllerConfig& InConfig = {});

    void Reset(float InCursor = 0.0f, float InSteering = 0.0f);
    float Step(
        float MouseDeltaX,
        bool bGazeHeld,
        float SpeedKmh,
        EPinkCabVehicleMotionMode MotionMode,
        float DeltaSeconds);

    float GetVirtualCursor() const;
    float GetTarget() const;
    float GetSteering() const;
    float GetResponseRate(float SpeedKmh, EPinkCabVehicleMotionMode MotionMode) const;

private:
    FPinkCabSteeringControllerConfig Config;
    float VirtualCursor = 0.0f;
    float Target = 0.0f;
    float Steering = 0.0f;
};
