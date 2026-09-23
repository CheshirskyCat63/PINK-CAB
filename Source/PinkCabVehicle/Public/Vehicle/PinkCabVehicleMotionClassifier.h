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

class PINKCABVEHICLE_API FPinkCabVehicleMotionClassifier
{
public:
    explicit FPinkCabVehicleMotionClassifier(const FPinkCabVehicleMotionClassifierConfig& InConfig = {});

    EPinkCabVehicleMotionMode GetMode() const;
    uint32 GetTransitionSerial() const;
    EPinkCabVehicleMotionMode Update(float SpeedKmh, float DeltaSeconds);

private:
    FPinkCabVehicleMotionClassifierConfig Config;
    EPinkCabVehicleMotionMode Mode = EPinkCabVehicleMotionMode::Stationary;
    float PendingSeconds = 0.0f;
    uint32 TransitionSerial = 0;
};
