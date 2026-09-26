#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"

namespace
{
void PopulateActuationTelemetry(
    UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleControlState& Controls,
    const FPinkCabEngineActuationResult& Actuation,
    const float EngineTorqueCurveNm,
    FPinkCabCausalActuationTelemetry& Out)
{
    Out = {};
    Out.HealthClampedControlThrottle01 =
        FMath::Clamp(Controls.Throttle, 0.0f, 1.0f);
    Out.EngineThrottlePreLimiter01 =
        Controls.GetResolvedEngineThrottlePreLimiter01();
    Out.EngineThrottleFinal01 =
        Controls.GetResolvedEngineThrottle01();
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
    Out.ExternalRearDriveTorquePerWheelNm =
        Controls.ExternalRearDriveTorquePerWheelNm;
    Out.DriveTorquePath =
        FMath::Abs(Controls.ExternalRearDriveTorquePerWheelNm) > KINDA_SMALL_NUMBER
            ? EPinkCabCausalDriveTorquePath::ExternalPartialClutch
            : (Controls.EngagedGear != 0 && Controls.ClutchCoupling >= 1.0f
                ? EPinkCabCausalDriveTorquePath::ChaosMechanical
                : EPinkCabCausalDriveTorquePath::None);
    Out.bTorqueControlEnabled = Movement.TorqueControl.Enabled;
    Out.bTargetRotationControlEnabled =
        Movement.TargetRotationControl.Enabled;
    Out.bStabilizeControlEnabled = Movement.StabilizeControl.Enabled;
    Out.AssistContribution = 0.0f;
}

void ApplyRearWheelTorques(
    UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleControlState& Controls)
{
    Movement.SetHandbrakeInput(false);
    constexpr float RearHandbrakeMaxTorqueNm = 1700.0f;
    const float RearBrakeTorqueNm =
        FMath::Clamp(Controls.Handbrake, 0.0f, 1.0f)
        * RearHandbrakeMaxTorqueNm;
    const int32 WheelCount = Movement.GetNumWheels();
    for (int32 WheelIndex = 2; WheelIndex < FMath::Min(WheelCount, 4); ++WheelIndex)
    {
        Movement.SetTorqueCombineMethod(
            ETorqueCombineMethod::Additive,
            WheelIndex);
        Movement.SetBrakeTorque(RearBrakeTorqueNm, WheelIndex);
        Movement.SetDriveTorque(
            Controls.ExternalRearDriveTorquePerWheelNm,
            WheelIndex);
    }
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
    Movement->SetSteeringInput(Controls.Steering);

    PopulateActuationTelemetry(
        *Movement,
        Controls,
        {},
        0.0f,
        LastCausalActuation);

    Movement->SetThrottleInput(
        Controls.GetResolvedEngineThrottle01());
    Movement->SetBrakeInput(Controls.Brake);
    ApplyRearWheelTorques(*Movement, Controls);
    PopulateConfiguredAssistFlags(*Movement, LastCausalActuation);
    return true;
}
