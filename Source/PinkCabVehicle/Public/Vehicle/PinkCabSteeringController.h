#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

struct FPinkCabSteeringControllerConfig
{
    float MouseCountsForFullScale = 1400.0f;
    float CenterExponent = 1.22f;
    float StationaryTravelScale = 3.00f;
    float MovingTravelScaleLow = 1.25f;
    float MovingTravelScaleHigh = 1.85f;
    float StationaryResponsePerSecond = 3.2f;
    float MovingResponseLowPerSecond = 13.0f;
    float MovingResponseHighPerSecond = 8.0f;
    float HighSpeedKmh = 120.0f;
    float HighSpeedTargetGain = 0.60f;
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
