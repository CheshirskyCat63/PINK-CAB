#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabGearboxController.h"
#include "Vehicle/PinkCabVehicleHealthState.h"

struct FPinkCabDrivetrainConditionInput
{
    float DeltaSeconds = 0.0f;
    bool bEngineRunning = false;
    float EngineRpm = 0.0f;
    float ExpectedCoupledRpm = 0.0f;
    float SpeedKmh = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Handbrake = 0.0f;
    float ClutchCoupling = 0.0f;
    int32 EngagedGear = 0;
};

struct FPinkCabDrivetrainConditionOutput
{
    bool bShouldStall = false;
    bool bEngineLugging = false;
    float EngineTorqueFactor = 1.0f;
    float DisplayedEngineRpm = 0.0f;
    float BrakeEffectiveness = 1.0f;
    float HandbrakeEffectiveness = 1.0f;
    float DrivetrainTorqueCapacity = 1.0f;
};

struct FPinkCabDrivetrainConditionConfig
{
    float ClutchHeatGainPerSecond = 0.30f;
    float ClutchCoolPerSecond = 0.05f;
    float ClutchWearPerSlipSecond = 0.00075f;
    float BrakeHeatGainPerSecond = 0.020f;
    float BrakeCoolPerSecond = 0.006f;
    float BrakeWearHotThreshold = 0.82f;
    float BrakeWearPerHotSecond = 0.00012f;
    float BrakeFadeStart = 0.65f;
    float BrakeFadeMinimum = 0.55f;
    float StallRpm = 850.0f;
    float LugStartRpm = 1000.0f;
    float LugStallRpm = 800.0f;
    float LugStallDelaySeconds = 1.80f;
    float LugPulseHz = 2.5f;
    float LugTorqueMinimum = 0.62f;
    float LugTorqueMaximum = 0.90f;
};

class PINKCABVEHICLE_API FPinkCabDrivetrainCondition
{
public:
    explicit FPinkCabDrivetrainCondition(const FPinkCabDrivetrainConditionConfig& InConfig = {});

    FPinkCabDrivetrainConditionOutput Step(
        const FPinkCabDrivetrainConditionInput& Input,
        FPinkCabVehicleHealthState& Health);
    void RecordGearEvent(
        EPinkCabGearEngagementResult Result,
        FPinkCabVehicleHealthState& Health) const;
    float GetBrakeEffectiveness(const FPinkCabVehicleHealthState& Health) const;
    float GetClutchTorqueCapacity(const FPinkCabVehicleHealthState& Health) const;

private:
    bool ShouldLaunchStall(const FPinkCabDrivetrainConditionInput& Input) const;
    bool IsLugging(const FPinkCabDrivetrainConditionInput& Input) const;
    bool ShouldLugStall(const FPinkCabDrivetrainConditionInput& Input, bool bLugging) const;
    void ApplyLugging(
        const FPinkCabDrivetrainConditionInput& Input,
        float Dt,
        FPinkCabDrivetrainConditionOutput& Output);
    void RecoverLugExposure(float Dt);
    void UpdateClutch(
        const FPinkCabDrivetrainConditionInput& Input,
        float Dt,
        FPinkCabVehicleHealthState& Health) const;
    void UpdateBrakes(
        const FPinkCabDrivetrainConditionInput& Input,
        float Dt,
        FPinkCabVehicleHealthState& Health) const;

    FPinkCabDrivetrainConditionConfig Config;
    float LugExposureSeconds = 0.0f;
};
