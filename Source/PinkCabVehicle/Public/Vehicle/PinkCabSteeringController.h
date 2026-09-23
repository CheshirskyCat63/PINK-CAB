#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

struct FPinkCabSteeringControllerConfig
{
    float MouseCountsForFullScale = 1400.0f;
    float CenterExponent = 1.28f;
    float StationaryTravelScale = 3.60f;
    float MovingTravelScaleLow = 1.35f;
    float MovingTravelScaleHigh = 2.20f;
    float StationaryResponsePerSecond = 2.5f;
    float MovingResponseLowPerSecond = 10.5f;
    float MovingResponseHighPerSecond = 6.0f;
    float HighSpeedKmh = 120.0f;
    float HighSpeedTargetGain = 0.55f;
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
