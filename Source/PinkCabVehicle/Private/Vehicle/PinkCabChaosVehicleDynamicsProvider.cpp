#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosEngineAdapter.h"

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
    Out.HealthClampedControlThrottle01 = FMath::Clamp(Controls.Throttle, 0.0f, 1.0f);
    Out.EngineThrottlePreLimiter01 = Controls.GetResolvedEngineThrottlePreLimiter01();
    Out.EngineThrottleFinal01 = Controls.GetResolvedEngineThrottle01();
    Out.ChaosThrottleInput01 = FPinkCabChaosEngineAdapter::ToChaosThrottleInput(
        Out.EngineThrottleFinal01);
    Out.EngineTorqueCurveNm = Controls.GetResolvedEngineTorqueCurveNm();
    Out.RequestedEngineTorqueAfterLimiterHealthNm = Controls.GetAvailableEngineTorqueNm();
    Out.EffectiveGearRatio = Controls.EngagedGear != 0
        ? Movement.TransmissionSetup.GetGearRatio(Controls.EngagedGear)
        : 0.0f;
    Out.ConfiguredFinalDriveRatio = Movement.TransmissionSetup.FinalRatio;
    Out.TransmissionEfficiency = Movement.TransmissionSetup.TransmissionEfficiency;
    Out.ExternalRearDriveTorquePerWheelNm = 0.0f;
    Out.DriveTorquePath = Controls.EngagedGear != 0
        ? EPinkCabCausalDriveTorquePath::ChaosMechanical
        : EPinkCabCausalDriveTorquePath::None;
    Out.bTorqueControlEnabled = Movement.TorqueControl.Enabled;
    Out.bTargetRotationControlEnabled = Movement.TargetRotationControl.Enabled;
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
        Out.bAnyTractionControlConfigured |= Wheel->bTractionControlEnabled;
    }
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

    LastControls = Controls;
    PopulateActuationTelemetry(*Movement, Controls, LastCausalActuation);

    Movement->SetUseAutomaticGears(false);
    Movement->SetSteeringInput(Controls.Steering);
    Movement->SetThrottleInput(
        Controls.IsCombustionAllowed()
            ? LastCausalActuation.ChaosThrottleInput01
            : 0.0f);
    Movement->SetBrakeInput(FMath::Clamp(Controls.Brake, 0.0f, 1.0f));
    Movement->SetHandbrakeInput(Controls.Handbrake > KINDA_SMALL_NUMBER);

    // Stock Chaos does not expose a continuous clutch input. Preserve the
    // physical H-pattern UX as a control layer: an open/slipping clutch maps
    // to neutral, and a released clutch hands the selected gear to Chaos.
    const bool bClutchReleased = Controls.ClutchCoupling >= 0.95f;
    const int32 NativeGear = bClutchReleased ? Controls.EngagedGear : 0;
    Movement->SetTargetGear(NativeGear, true);

    PopulateConfiguredAssistFlags(*Movement, LastCausalActuation);
    return true;
}
