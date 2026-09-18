#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

struct FPinkCabHandbrakeActuatorConfig
{
    float MouseCountsForFullPull = 420.0f;
    float TorqueExponent = 1.65f;
    float AutoReturnPerSecond = 4.5f;
    float InitialParkingAmount = 1.0f;
};

class FPinkCabHandbrakeActuator
{
public:
    explicit FPinkCabHandbrakeActuator(
        const FPinkCabHandbrakeActuatorConfig& InConfig = {})
        : Config(InConfig)
    {
        Reset(InConfig.InitialParkingAmount, InConfig.InitialParkingAmount > KINDA_SMALL_NUMBER);
    }

    void Reset(float InLeverPosition, bool bLatched)
    {
        LeverPosition = FMath::Clamp(InLeverPosition, 0.0f, 1.0f);
        bParkingLatched = bLatched;
        bGripWasHeld = false;
        BrakeCommand = EvaluateTorqueCommand(LeverPosition);
    }
    float Step(
        EPinkCabVehicleMotionMode MotionMode,
        bool bGripHeld,
        float PullMouseDelta,
        float DeltaSeconds)
    {
        const float SafeCounts = FMath::Max(Config.MouseCountsForFullPull, 1.0f);

        if (MotionMode == EPinkCabVehicleMotionMode::Stationary)
        {
            if (bGripHeld)
            {
                LeverPosition = FMath::Clamp(
                    LeverPosition + PullMouseDelta / SafeCounts,
                    0.0f,
                    1.0f);
                bParkingLatched = false;
            }
            else if (bGripWasHeld)
            {
                bParkingLatched = LeverPosition > KINDA_SMALL_NUMBER;
            }
        }
        else
        {
            bParkingLatched = false;
            if (bGripHeld)
            {
                LeverPosition = FMath::Clamp(
                    LeverPosition + PullMouseDelta / SafeCounts,
                    0.0f,
                    1.0f);
            }
            else if (DeltaSeconds > 0.0f)
            {
                LeverPosition = FMath::FInterpConstantTo(
                    LeverPosition,
                    0.0f,
                    DeltaSeconds,
                    FMath::Max(Config.AutoReturnPerSecond, KINDA_SMALL_NUMBER));
            }
        }

        bGripWasHeld = bGripHeld;
        BrakeCommand = EvaluateTorqueCommand(LeverPosition);
        return BrakeCommand;
    }

    float GetLeverPosition() const { return LeverPosition; }
    float GetBrakeCommand() const { return BrakeCommand; }
    bool IsParkingLatched() const { return bParkingLatched; }

private:
    float EvaluateTorqueCommand(float Position) const
    {
        return FMath::Pow(
            FMath::Clamp(Position, 0.0f, 1.0f),
            FMath::Max(Config.TorqueExponent, 1.0f));
    }

    FPinkCabHandbrakeActuatorConfig Config;
    float LeverPosition = 1.0f;
    float BrakeCommand = 1.0f;
    bool bParkingLatched = true;
    bool bGripWasHeld = false;
};
