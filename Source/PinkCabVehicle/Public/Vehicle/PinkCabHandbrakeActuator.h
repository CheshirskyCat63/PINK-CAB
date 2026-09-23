#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

struct FPinkCabHandbrakeActuatorConfig
{
    float MouseCountsForFullPull = 220.0f;
    float TorqueExponent = 1.65f;
    float AutoReturnPerSecond = 4.5f;
    float InitialParkingAmount = 1.0f;
};

class PINKCABVEHICLE_API FPinkCabHandbrakeActuator
{
public:
    explicit FPinkCabHandbrakeActuator(const FPinkCabHandbrakeActuatorConfig& InConfig = {});

    void Reset(float InLeverPosition, bool bLatched);
    float Step(
        EPinkCabVehicleMotionMode MotionMode,
        bool bGripHeld,
        float PullMouseDelta,
        float DeltaSeconds);

    float GetLeverPosition() const;
    float GetBrakeCommand() const;
    bool IsParkingLatched() const;

private:
    float EvaluateTorqueCommand(float Position) const;

    FPinkCabHandbrakeActuatorConfig Config;
    float LeverPosition = 1.0f;
    float BrakeCommand = 1.0f;
    bool bParkingLatched = true;
    bool bGripWasHeld = false;
};
