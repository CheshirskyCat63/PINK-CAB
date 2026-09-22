#include "Vehicle/PinkCabSteeringController.h"

FPinkCabSteeringController::FPinkCabSteeringController(
    const FPinkCabSteeringControllerConfig& InConfig)
    : Config(InConfig)
{
}

void FPinkCabSteeringController::Reset(float InCursor, float InSteering)
{
    VirtualCursor = FMath::Clamp(InCursor, -1.0f, 1.0f);
    Steering = FMath::Clamp(InSteering, -1.0f, 1.0f);
    Target = Steering;
}

float FPinkCabSteeringController::Step(
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

    const float SpeedAlpha = FMath::Clamp(
        FMath::Abs(SpeedKmh) / FMath::Max(Config.HighSpeedKmh, 1.0f),
        0.0f,
        1.0f);
    const float TravelScale =
        MotionMode == EPinkCabVehicleMotionMode::Stationary
            ? FMath::Max(Config.StationaryTravelScale, 1.0f)
            : FMath::Lerp(
                FMath::Max(Config.MovingTravelScaleLow, 1.0f),
                FMath::Max(Config.MovingTravelScaleHigh, 1.0f),
                SpeedAlpha);
    const float Counts =
        FMath::Max(Config.MouseCountsForFullScale * TravelScale, 1.0f);
    VirtualCursor = FMath::Clamp(
        VirtualCursor + MouseDeltaX / Counts,
        -1.0f,
        1.0f);

    const float AbsCursor = FMath::Abs(VirtualCursor);
    const float Curve = FMath::Pow(
        AbsCursor,
        FMath::Max(Config.CenterExponent, 1.0f));
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

float FPinkCabSteeringController::GetVirtualCursor() const
{
    return VirtualCursor;
}

float FPinkCabSteeringController::GetTarget() const
{
    return Target;
}

float FPinkCabSteeringController::GetSteering() const
{
    return Steering;
}

float FPinkCabSteeringController::GetResponseRate(
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
