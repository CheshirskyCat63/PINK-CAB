#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

struct FPinkCabSteeringControllerConfig
{
    float MouseCountsForFullScale = 1400.0f;
    float CenterExponent = 1.65f;
    float StationaryResponsePerSecond = 0.65f;
    float MovingResponseLowPerSecond = 2.25f;
    float MovingResponseHighPerSecond = 4.50f;
    float HighSpeedKmh = 160.0f;
    float HighSpeedTargetGain = 1.12f;
};

class FPinkCabSteeringController
{
public:
    explicit FPinkCabSteeringController(
        const FPinkCabSteeringControllerConfig& InConfig = {})
        : Config(InConfig)
    {
    }

    void Reset(float InCursor = 0.0f, float InSteering = 0.0f)
    {
        VirtualCursor = FMath::Clamp(InCursor, -1.0f, 1.0f);
        Steering = FMath::Clamp(InSteering, -1.0f, 1.0f);
        Target = Steering;
    }
    float Step(
        float MouseDeltaX,
        bool bGazeHeld,
        float SpeedKmh,
        EPinkCabVehicleMotionMode MotionMode,
        float DeltaSeconds)
    {
        if (bGazeHeld)
        {
            return Steering;
        }

        const float Counts = FMath::Max(Config.MouseCountsForFullScale, 1.0f);
        VirtualCursor = FMath::Clamp(
            VirtualCursor + MouseDeltaX / Counts,
            -1.0f,
            1.0f);

        const float AbsCursor = FMath::Abs(VirtualCursor);
        const float Curve = FMath::Pow(
            AbsCursor,
            FMath::Max(Config.CenterExponent, 1.0f));
        const float SpeedAlpha = FMath::Clamp(
            FMath::Abs(SpeedKmh) / FMath::Max(Config.HighSpeedKmh, 1.0f),
            0.0f,
            1.0f);
        const float Gain = MotionMode == EPinkCabVehicleMotionMode::Moving
            ? FMath::Lerp(1.0f, Config.HighSpeedTargetGain, SpeedAlpha)
            : 1.0f;
        Target = FMath::Clamp(
            FMath::Sign(VirtualCursor) * Curve * Gain,
            -1.0f,
            1.0f);

        if (DeltaSeconds <= 0.0f)
        {
            return Steering;
        }

        const float ResponseRate = GetResponseRate(SpeedKmh, MotionMode);
        Steering = FMath::FInterpConstantTo(
            Steering,
            Target,
            DeltaSeconds,
            ResponseRate);
        Steering = FMath::Clamp(Steering, -1.0f, 1.0f);
        return Steering;
    }

    float GetVirtualCursor() const { return VirtualCursor; }
    float GetTarget() const { return Target; }
    float GetSteering() const { return Steering; }
    float GetResponseRate(
        float SpeedKmh,
        EPinkCabVehicleMotionMode MotionMode) const
    {
        if (MotionMode == EPinkCabVehicleMotionMode::Stationary)
        {
            return Config.StationaryResponsePerSecond;
        }

        const float SpeedAlpha = FMath::Clamp(
            FMath::Abs(SpeedKmh) / FMath::Max(Config.HighSpeedKmh, 1.0f),
            0.0f,
            1.0f);
        return FMath::Lerp(
            Config.MovingResponseLowPerSecond,
            Config.MovingResponseHighPerSecond,
            SpeedAlpha);
    }

private:
    FPinkCabSteeringControllerConfig Config;
    float VirtualCursor = 0.0f;
    float Target = 0.0f;
    float Steering = 0.0f;
};
