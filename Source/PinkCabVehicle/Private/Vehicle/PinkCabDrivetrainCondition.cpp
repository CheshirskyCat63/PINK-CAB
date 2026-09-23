#include "Vehicle/PinkCabDrivetrainCondition.h"

FPinkCabDrivetrainCondition::FPinkCabDrivetrainCondition(
    const FPinkCabDrivetrainConditionConfig& InConfig)
    : Config(InConfig)
{
}

FPinkCabDrivetrainConditionOutput FPinkCabDrivetrainCondition::Step(
    const FPinkCabDrivetrainConditionInput& Input,
    FPinkCabVehicleHealthState& Health)
{
    const float Dt = FMath::Clamp(Input.DeltaSeconds, 0.0f, 1.0f);
    UpdateClutch(Input, Dt, Health);
    UpdateBrakes(Input, Dt, Health);

    FPinkCabDrivetrainConditionOutput Output;
    Output.DisplayedEngineRpm =
        Input.bEngineRunning ? FMath::Max(Input.EngineRpm, 0.0f) : 0.0f;

    const bool bLaunchStall = ShouldLaunchStall(Input);
    const bool bLugging = IsLugging(Input);
    if (bLugging) ApplyLugging(Input, Dt, Output);
    else RecoverLugExposure(Dt);

    Output.bShouldStall = bLaunchStall || ShouldLugStall(Input, bLugging);
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

bool FPinkCabDrivetrainCondition::ShouldLaunchStall(
    const FPinkCabDrivetrainConditionInput& Input) const
{
    return Input.bEngineRunning
        && Input.EngagedGear != 0
        && Input.ClutchCoupling > 0.80f
        && Input.EngineRpm > 0.0f
        && Input.EngineRpm < Config.StallRpm
        && FMath::Abs(Input.SpeedKmh) < 3.0f
        && Input.Throttle < 0.12f;
}

bool FPinkCabDrivetrainCondition::IsLugging(
    const FPinkCabDrivetrainConditionInput& Input) const
{
    const bool bMovingFullyCoupled =
        Input.bEngineRunning
        && Input.EngagedGear != 0
        && Input.ClutchCoupling > 0.80f
        && FMath::Abs(Input.SpeedKmh) >= 3.0f;
    return bMovingFullyCoupled
        && Input.EngagedGear >= 3
        && Input.ExpectedCoupledRpm > 0.0f
        && Input.ExpectedCoupledRpm < Config.LugStartRpm;
}

bool FPinkCabDrivetrainCondition::ShouldLugStall(
    const FPinkCabDrivetrainConditionInput& Input,
    bool bLugging) const
{
    return bLugging
        && Input.ExpectedCoupledRpm <= Config.LugStallRpm
        && LugExposureSeconds >= Config.LugStallDelaySeconds;
}

void FPinkCabDrivetrainCondition::ApplyLugging(
    const FPinkCabDrivetrainConditionInput& Input,
    float Dt,
    FPinkCabDrivetrainConditionOutput& Output)
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
        CoupledRpm * FMath::Lerp(1.0f, 0.72f, Exposure01 * Severity);
}

void FPinkCabDrivetrainCondition::RecoverLugExposure(float Dt)
{
    LugExposureSeconds = FMath::Max(0.0f, LugExposureSeconds - Dt * 2.0f);
}

void FPinkCabDrivetrainCondition::RecordGearEvent(
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

float FPinkCabDrivetrainCondition::GetBrakeEffectiveness(
    const FPinkCabVehicleHealthState& Health) const
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

float FPinkCabDrivetrainCondition::GetClutchTorqueCapacity(
    const FPinkCabVehicleHealthState& Health) const
{
    const float HealthFactor = Health.GetHealth(EPinkCabVehicleHealthChannel::Clutch);
    const float HeatPenalty = FMath::Lerp(1.0f, 0.70f, Health.GetClutchTemperature01());
    return FMath::Clamp(HealthFactor * HeatPenalty, 0.0f, 1.0f);
}

void FPinkCabDrivetrainCondition::UpdateClutch(
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

void FPinkCabDrivetrainCondition::UpdateBrakes(
    const FPinkCabDrivetrainConditionInput& Input,
    float Dt,
    FPinkCabVehicleHealthState& Health) const
{
    float Heat = Health.GetBrakeTemperature01();
    const float WheelSpeed01 =
        FMath::Clamp(FMath::Abs(Input.SpeedKmh) / 100.0f, 0.0f, 1.5f);
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
