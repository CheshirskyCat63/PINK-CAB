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
    // Speed may shape how new mouse travel reaches the authored steering target,
    // but it must never rewrite an already-authored target merely because vehicle
    // speed changed. Full mechanical steering authority therefore remains available
    // at every speed; high-speed calmness comes only from the input travel scale above.
    Target = FMath::Clamp(
        FMath::Sign(VirtualCursor) * Curve,
        -1.0f,
        1.0f);

    // Mouse delta already authors the persistent steering target through
    // VirtualCursor. A second temporal interpolation layer would keep moving the
    // command after the device delta has ended, which is undeclared ghost steering.
    // Standstill/high-speed feel remains in the device travel scale above.
    (void)DeltaSeconds;
    Steering = Target;
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
