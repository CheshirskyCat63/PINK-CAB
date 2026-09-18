#pragma once

#include "CoreMinimal.h"

enum class EPinkCabVehicleMotionMode : uint8
{
    Stationary,
    Moving
};

struct FPinkCabVehicleMotionClassifierConfig
{
    float EnterMovingKmh = 1.5f;
    float EnterStationaryKmh = 0.5f;
    float DebounceSeconds = 0.12f;
};

class FPinkCabVehicleMotionClassifier
{
public:
    explicit FPinkCabVehicleMotionClassifier(
        const FPinkCabVehicleMotionClassifierConfig& InConfig = {})
        : Config(InConfig)
    {
    }

    EPinkCabVehicleMotionMode GetMode() const { return Mode; }
    uint32 GetTransitionSerial() const { return TransitionSerial; }

    EPinkCabVehicleMotionMode Update(float SpeedKmh, float DeltaSeconds)
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

private:
    FPinkCabVehicleMotionClassifierConfig Config;
    EPinkCabVehicleMotionMode Mode = EPinkCabVehicleMotionMode::Stationary;
    float PendingSeconds = 0.0f;
    uint32 TransitionSerial = 0;
};
