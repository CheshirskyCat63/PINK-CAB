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
    // Chaos keeps a running engine at the physical profile's 750 RPM idle floor.
    // Keep the stall decision slightly above that floor so a clutch dump can actually
    // stop the engine instead of requiring an unreachable sub-idle telemetry value.
    float StallRpm = 850.0f;
    // A fully-coupled high gear below this range must lug instead of behaving
    // like an electric motor. Chaos itself holds a 750 RPM idle floor, so this
    // state supplies the missing combustion-engine load behavior.
    float LugStartRpm = 1000.0f;
    float LugStallRpm = 800.0f;
    float LugStallDelaySeconds = 0.90f;
    float LugPulseHz = 4.0f;
    float LugTorqueMinimum = 0.45f;
    float LugTorqueMaximum = 0.72f;
};

class FPinkCabDrivetrainCondition
{
public:
    explicit FPinkCabDrivetrainCondition(
        const FPinkCabDrivetrainConditionConfig& InConfig = {})
        : Config(InConfig)
    {
    }

    FPinkCabDrivetrainConditionOutput Step(
        const FPinkCabDrivetrainConditionInput& Input,
        FPinkCabVehicleHealthState& Health)
    {
        const float Dt = FMath::Clamp(Input.DeltaSeconds, 0.0f, 1.0f);
        UpdateClutch(Input, Dt, Health);
        UpdateBrakes(Input, Dt, Health);

        FPinkCabDrivetrainConditionOutput Output;
        Output.DisplayedEngineRpm =
            Input.bEngineRunning ? FMath::Max(Input.EngineRpm, 0.0f) : 0.0f;

        const bool bLaunchStall =
            Input.bEngineRunning
            && Input.EngagedGear != 0
            && Input.ClutchCoupling > 0.80f
            && Input.EngineRpm > 0.0f
            && Input.EngineRpm < Config.StallRpm
            && FMath::Abs(Input.SpeedKmh) < 3.0f
            && Input.Throttle < 0.12f;

        const bool bMovingFullyCoupled =
            Input.bEngineRunning
            && Input.EngagedGear != 0
            && Input.ClutchCoupling > 0.80f
            && FMath::Abs(Input.SpeedKmh) >= 3.0f;
        const bool bLugging =
            bMovingFullyCoupled
            && Input.ExpectedCoupledRpm > 0.0f
            && Input.ExpectedCoupledRpm < Config.LugStartRpm;

        if (bLugging)
        {
            LugExposureSeconds += Dt;
            const float Severity = FMath::Clamp(
                (Config.LugStartRpm - Input.ExpectedCoupledRpm)
                    / FMath::Max(Config.LugStartRpm - Config.LugStallRpm, 1.0f),
                0.0f,
                1.0f);
            const float Pulse01 =
                0.5f + 0.5f * FMath::Sin(LugExposureSeconds * 2.0f * PI * Config.LugPulseHz);
            const float LugTorque =
                FMath::Lerp(Config.LugTorqueMinimum, Config.LugTorqueMaximum, Pulse01);
            Output.bEngineLugging = true;
            Output.EngineTorqueFactor = FMath::Lerp(1.0f, LugTorque, Severity);

            const float Exposure01 = FMath::Clamp(
                LugExposureSeconds / FMath::Max(Config.LugStallDelaySeconds, KINDA_SMALL_NUMBER),
                0.0f,
                1.0f);
            const float CoupledRpm =
                FMath::Min(Output.DisplayedEngineRpm, Input.ExpectedCoupledRpm);
            Output.DisplayedEngineRpm =
                CoupledRpm * FMath::Lerp(1.0f, 0.65f, Exposure01 * Severity);
        }
        else
        {
            LugExposureSeconds = FMath::Max(0.0f, LugExposureSeconds - Dt * 2.0f);
        }

        const bool bLugStall =
            bLugging
            && Input.ExpectedCoupledRpm <= Config.LugStallRpm
            && LugExposureSeconds >= Config.LugStallDelaySeconds;
        Output.bShouldStall = bLaunchStall || bLugStall;
        if (Output.bShouldStall)
        {
            Output.DisplayedEngineRpm = 0.0f;
        }

        Output.BrakeEffectiveness = GetBrakeEffectiveness(Health);
        Output.HandbrakeEffectiveness = Output.BrakeEffectiveness;
        Output.DrivetrainTorqueCapacity =
            GetClutchTorqueCapacity(Health)
            * Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox);
        return Output;
    }

    void RecordGearEvent(
        EPinkCabGearEngagementResult Result,
        FPinkCabVehicleHealthState& Health) const
    {
        if (Result == EPinkCabGearEngagementResult::GrindRefused)
        {
            Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Gearbox, 0.0025f);
        }
        else if (Result == EPinkCabGearEngagementResult::DangerousOverrev)
        {
            Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Gearbox, 0.0125f);
        }
    }

    float GetBrakeEffectiveness(const FPinkCabVehicleHealthState& Health) const
    {
        const float HealthFactor = Health.GetHealth(EPinkCabVehicleHealthChannel::Brake);
        const float Temperature = Health.GetBrakeTemperature01();
        float Fade = 1.0f;
        if (Temperature > Config.BrakeFadeStart)
        {
            const float Alpha = FMath::Clamp(
                (Temperature - Config.BrakeFadeStart)
                    / FMath::Max(1.0f - Config.BrakeFadeStart, KINDA_SMALL_NUMBER),
                0.0f,
                1.0f);
            Fade = FMath::Lerp(1.0f, Config.BrakeFadeMinimum, Alpha);
        }
        return FMath::Clamp(HealthFactor * Fade, 0.0f, 1.0f);
    }

    float GetClutchTorqueCapacity(const FPinkCabVehicleHealthState& Health) const
    {
        const float HealthFactor = Health.GetHealth(EPinkCabVehicleHealthChannel::Clutch);
        const float HeatPenalty = FMath::Lerp(1.0f, 0.70f, Health.GetClutchTemperature01());
        return FMath::Clamp(HealthFactor * HeatPenalty, 0.0f, 1.0f);
    }

private:
    void UpdateClutch(
        const FPinkCabDrivetrainConditionInput& Input,
        float Dt,
        FPinkCabVehicleHealthState& Health) const
    {
        const bool bCanSlip =
            Input.bEngineRunning
            && Input.EngagedGear != 0
            && Input.ClutchCoupling > KINDA_SMALL_NUMBER
            && Input.ClutchCoupling < 0.995f;

        float Heat = Health.GetClutchTemperature01();
        if (bCanSlip)
        {
            const float RpmMismatch01 = FMath::Clamp(
                FMath::Abs(Input.EngineRpm - Input.ExpectedCoupledRpm) / 4000.0f,
                0.0f,
                1.0f);
            const float Load01 = FMath::Clamp(
                0.20f + FMath::Abs(Input.Throttle) * 0.80f,
                0.0f,
                1.0f);
            const float SlipIntensity =
                RpmMismatch01
                * Load01
                * FMath::Sin(PI * FMath::Clamp(Input.ClutchCoupling, 0.0f, 1.0f));

            Heat += SlipIntensity * Config.ClutchHeatGainPerSecond * Dt;
            if (SlipIntensity > KINDA_SMALL_NUMBER)
            {
                Health.ApplyFunctionalDamage(
                    EPinkCabVehicleHealthChannel::Clutch,
                    SlipIntensity * Config.ClutchWearPerSlipSecond * Dt);
            }
        }
        else
        {
            Heat -= Config.ClutchCoolPerSecond * Dt;
        }
        Health.SetClutchTemperature01(Heat);
    }

    void UpdateBrakes(
        const FPinkCabDrivetrainConditionInput& Input,
        float Dt,
        FPinkCabVehicleHealthState& Health) const
    {
        float Heat = Health.GetBrakeTemperature01();
        const float WheelSpeed01 = FMath::Clamp(FMath::Abs(Input.SpeedKmh) / 100.0f, 0.0f, 1.5f);
        const float BrakeDemand = FMath::Clamp(
            FMath::Max(FMath::Abs(Input.Brake), FMath::Abs(Input.Handbrake)),
            0.0f,
            1.0f);
        const float Heating = WheelSpeed01 * BrakeDemand * Config.BrakeHeatGainPerSecond * Dt;
        Heat += Heating;
        if (Heating <= KINDA_SMALL_NUMBER)
        {
            Heat -= Config.BrakeCoolPerSecond * Dt;
        }

        Health.SetBrakeTemperature01(Heat);
        if (Health.GetBrakeTemperature01() > Config.BrakeWearHotThreshold
            && BrakeDemand > 0.5f)
        {
            Health.ApplyFunctionalDamage(
                EPinkCabVehicleHealthChannel::Brake,
                Config.BrakeWearPerHotSecond * Dt);
        }
    }

    FPinkCabDrivetrainConditionConfig Config;
    float LugExposureSeconds = 0.0f;
};
