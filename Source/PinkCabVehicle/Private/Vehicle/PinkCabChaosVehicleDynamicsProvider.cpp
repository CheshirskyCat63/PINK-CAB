#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosEngineAdapter.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"

namespace
{
void PopulateActuationTelemetry(
    UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleControlState& Controls,
    FPinkCabCausalActuationTelemetry& Out)
{
    Out = {};
    Out.HealthClampedControlThrottle01 =
        FMath::Clamp(Controls.Throttle, 0.0f, 1.0f);
    Out.EngineThrottlePreLimiter01 =
        Controls.GetResolvedEngineThrottlePreLimiter01();
    Out.EngineThrottleFinal01 =
        Controls.GetResolvedEngineThrottle01();
    Out.ChaosThrottleInput01 =
        FPinkCabChaosEngineAdapter::ToChaosThrottleInput(
            Out.EngineThrottleFinal01);
    Out.EngineTorqueCurveNm =
        Controls.GetResolvedEngineTorqueCurveNm();
    Out.RequestedEngineTorqueAfterLimiterHealthNm =
        Controls.GetAvailableEngineTorqueNm();
    Out.EffectiveGearRatio =
        Controls.EngagedGear != 0
            ? Movement.TransmissionSetup.GetGearRatio(Controls.EngagedGear)
            : 0.0f;
    Out.ConfiguredFinalDriveRatio = Movement.TransmissionSetup.FinalRatio;
    Out.TransmissionEfficiency =
        Movement.TransmissionSetup.TransmissionEfficiency;

    // PHY-009: the legacy external-partial torque transport remains present
    // only for compatibility/evidence and is always zero on the production path.
    Out.ExternalRearDriveTorquePerWheelNm = 0.0f;
    Out.DriveTorquePath =
        Controls.EngagedGear != 0
            && Controls.ClutchCoupling > KINDA_SMALL_NUMBER
            ? EPinkCabCausalDriveTorquePath::PinkCabClutchDriveline
            : EPinkCabCausalDriveTorquePath::None;

    Out.bTorqueControlEnabled = Movement.TorqueControl.Enabled;
    Out.bTargetRotationControlEnabled =
        Movement.TargetRotationControl.Enabled;
    Out.bStabilizeControlEnabled = Movement.StabilizeControl.Enabled;
    Out.AssistContribution = 0.0f;
}

void PopulateConfiguredAssistFlags(
    const UChaosWheeledVehicleMovementComponent& Movement,
    FPinkCabCausalActuationTelemetry& Out)
{
    Out.bAnyAbsConfigured = false;
    Out.bAnyTractionControlConfigured = false;
    for (const UChaosVehicleWheel* Wheel : Movement.Wheels)
    {
        if (!Wheel)
        {
            continue;
        }
        Out.bAnyAbsConfigured |= Wheel->bABSEnabled;
        Out.bAnyTractionControlConfigured |=
            Wheel->bTractionControlEnabled;
    }
}

FPinkCabChaosDrivelineCommand BuildDrivelineCommand(
    UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleControlState& Controls)
{
    FPinkCabChaosDrivelineCommand Command;
    Command.bCombustionAllowed = Controls.IsCombustionAllowed();
    Command.RequestedGear = Controls.RequestedGear;
    Command.EngagedGear = Controls.EngagedGear;
    Command.ClutchCoupling01 = Controls.ClutchCoupling;
    Command.DrivetrainTorqueCapacity01 =
        Controls.DrivetrainTorqueCapacity;
    Command.HealthClampedControlThrottle01 =
        FMath::Clamp(Controls.Throttle, 0.0f, 1.0f);
    Command.ServiceBrake01 =
        FMath::Clamp(Controls.Brake, 0.0f, 1.0f);
    Command.EffectiveGearRatio =
        Controls.EngagedGear != 0
            ? Movement.TransmissionSetup.GetGearRatio(Controls.EngagedGear)
            : 0.0f;
    Command.TransmissionEfficiency =
        Movement.TransmissionSetup.TransmissionEfficiency;
    Command.EngineBrakeEffect =
        Movement.EngineSetup.EngineBrakeEffect;
    Command.Handbrake01 = Controls.Handbrake;
    return Command;
}
}

FPinkCabChaosVehicleDynamicsProvider::FPinkCabChaosVehicleDynamicsProvider(
    UChaosWheeledVehicleMovementComponent* InMovement)
    : Movement(InMovement)
{
}

bool FPinkCabChaosVehicleDynamicsProvider::ApplyControls(
    const FPinkCabVehicleControlState& Controls)
{
    if (!Movement)
    {
        return false;
    }

    UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
        Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
    if (!PinkCabMovement)
    {
        // Production PHY-009 must never silently fall back to the old
        // one-way external torque / native full-coupling split.
        return false;
    }

    LastControls = Controls;
    Movement->SetSteeringInput(Controls.Steering);

    PopulateActuationTelemetry(
        *Movement,
        Controls,
        LastCausalActuation);

    // Keep public Chaos inputs coherent for telemetry and generic input-rate
    // bookkeeping. The physics-thread simulation consumes the authoritative
    // clutch command below for actual engine/wheel torque exchange.
    Movement->SetThrottleInput(
        LastCausalActuation.ChaosThrottleInput01);
    Movement->SetBrakeInput(Controls.Brake);

    const FPinkCabChaosDrivelineCommand Command =
        BuildDrivelineCommand(*Movement, Controls);
    if (!PinkCabMovement->SetPinkCabDrivelineCommand(Command))
    {
        return false;
    }

    PopulateConfiguredAssistFlags(*Movement, LastCausalActuation);
    return true;
}
