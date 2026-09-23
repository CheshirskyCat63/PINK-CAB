#include "Vehicle/PinkCabVehicleMotionClassifier.h"

FPinkCabVehicleMotionClassifier::FPinkCabVehicleMotionClassifier(
    const FPinkCabVehicleMotionClassifierConfig& InConfig)
    : Config(InConfig)
{
}

EPinkCabVehicleMotionMode FPinkCabVehicleMotionClassifier::GetMode() const
{
    return Mode;
}

uint32 FPinkCabVehicleMotionClassifier::GetTransitionSerial() const
{
    return TransitionSerial;
}

EPinkCabVehicleMotionMode FPinkCabVehicleMotionClassifier::Update(
    float SpeedKmh,
    float DeltaSeconds)
{
    const float Speed = FMath::Abs(SpeedKmh);
    const bool bWantsMoving =
        Mode == EPinkCabVehicleMotionMode::Stationary && Speed >= Config.EnterMovingKmh;
    const bool bWantsStationary =
        Mode == EPinkCabVehicleMotionMode::Moving && Speed <= Config.EnterStationaryKmh;
    const bool bWantsTransition = bWantsMoving || bWantsStationary;

    if (!bWantsTransition)
    {
        PendingSeconds = 0.0f;
        return Mode;
    }

    PendingSeconds += FMath::Max(DeltaSeconds, 0.0f);
    if (PendingSeconds + KINDA_SMALL_NUMBER < Config.DebounceSeconds)
    {
        return Mode;
    }

    Mode = bWantsMoving
        ? EPinkCabVehicleMotionMode::Moving
        : EPinkCabVehicleMotionMode::Stationary;
    PendingSeconds = 0.0f;
    ++TransitionSerial;
    return Mode;
}
