#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

struct FPinkCabSteeringControllerConfig
{
    float MouseCountsForFullScale = 1400.0f;
    // Keep the center close to linear: the driver's physical mouse circle already
    // provides precision, so the steering curve must not manufacture a dead zone.
    float CenterExponent = 1.10f;
    // Response is intentionally heavier at rest and progressively quicker in motion.
    // These are exponential response rates, not fixed steering-units-per-second.
    float StationaryResponsePerSecond = 3.0f;
    float MovingResponseLowPerSecond = 8.0f;
    float MovingResponseHighPerSecond = 12.0f;
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
    static float ResolveHorizontalMouseDelta(
        const float ProcessedMouseX,
        const float RawMouseX)
    {
        if (!FMath::IsFinite(ProcessedMouseX) || !FMath::IsFinite(RawMouseX))
        {
            return 0.0f;
        }

        // Raw X preserves the authored physical steering workspace, but it can
        // remain cached while only MouseY moves. Require horizontal motion in
        // this frame before accepting the raw X magnitude.
        return FMath::IsNearlyZero(ProcessedMouseX, 0.0001f) ? 0.0f : RawMouseX;
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

        const float ResponseRate = FMath::Max(GetResponseRate(SpeedKmh, MotionMode), 0.0f);
        const float ResponseAlpha = FMath::Clamp(
            1.0f - FMath::Exp(-ResponseRate * DeltaSeconds),
            0.0f,
            1.0f);
        Steering = FMath::Lerp(Steering, Target, ResponseAlpha);
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
