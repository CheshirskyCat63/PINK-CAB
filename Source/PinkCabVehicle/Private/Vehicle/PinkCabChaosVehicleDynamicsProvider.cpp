#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"

namespace
{
float GetEngineTorqueCurveNm(
    const UChaosWheeledVehicleMovementComponent& Movement,
    const float EngineRpm)
{
    const float NormalizedTorque =
        Movement.EngineSetup.TorqueCurve.GetRichCurveConst()->Eval(EngineRpm);
    return Movement.EngineSetup.MaxTorque
        * FMath::Max(NormalizedTorque, 0.0f);
}

FPinkCabEngineActuationResult ResolveEngineActuation(
    const UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleControlState& Controls,
    const float EngineRpm,
    const float EngineTorqueCurveNm)
{
    FPinkCabEngineActuationInput Input;
    Input.HealthClampedControlThrottle01 = Controls.Throttle;
    Input.EngineRpm = EngineRpm;
    Input.MaxRpm = Movement.EngineSetup.MaxRPM;
    Input.EngineTorqueCurveNm = EngineTorqueCurveNm;
    return FPinkCabEngineActuationResolver::Resolve(Input);
}

void PopulateActuationTelemetry(
    const UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleControlState& Controls,
    const FPinkCabEngineActuationResult& Actuation,
    const float EngineTorqueCurveNm,
    FPinkCabCausalActuationTelemetry& Out)
{
    Out = {};
    Out.HealthClampedControlThrottle01 =
        FMath::Clamp(Controls.Throttle, 0.0f, 1.0f);
    Out.EngineThrottlePreLimiter01 =
        Actuation.EngineThrottlePreLimiter01;
    Out.EngineThrottleFinal01 = Actuation.EngineThrottleFinal01;
    Out.EngineTorqueCurveNm = EngineTorqueCurveNm;
    Out.RequestedEngineTorqueAfterLimiterHealthNm =
        Actuation.RequestedEngineTorqueAfterLimiterHealthNm;
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

    const float EngineRpm = Movement->GetEngineRotationSpeed();
    const float EngineTorqueCurveNm =
        GetEngineTorqueCurveNm(*Movement, EngineRpm);
    const FPinkCabEngineActuationResult Actuation =
        ResolveEngineActuation(
            *Movement,
            Controls,
            EngineRpm,
            EngineTorqueCurveNm);
    PopulateActuationTelemetry(
        *Movement,
        Controls,
        Actuation,
        EngineTorqueCurveNm,
        LastCausalActuation);

    Movement->SetThrottleInput(Actuation.EngineThrottleFinal01);
    Movement->SetBrakeInput(Controls.Brake);
    ApplyRearWheelTorques(*Movement, Controls);
    PopulateConfiguredAssistFlags(*Movement, LastCausalActuation);
    return true;
}
