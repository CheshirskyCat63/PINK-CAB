#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"

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
    const float NormalizedTorque =
        Movement->EngineSetup.TorqueCurve.GetRichCurveConst()->Eval(EngineRpm);
    const float EngineTorqueCurveNm =
        Movement->EngineSetup.MaxTorque * FMath::Max(NormalizedTorque, 0.0f);

    FPinkCabEngineActuationInput ActuationInput;
    ActuationInput.HealthClampedControlThrottle01 = Controls.Throttle;
    ActuationInput.EngineRpm = EngineRpm;
    ActuationInput.MaxRpm = Movement->EngineSetup.MaxRPM;
    ActuationInput.EngineTorqueCurveNm = EngineTorqueCurveNm;
    const FPinkCabEngineActuationResult Actuation =
        FPinkCabEngineActuationResolver::Resolve(ActuationInput);

    LastCausalActuation = {};
    LastCausalActuation.HealthClampedControlThrottle01 =
        FMath::Clamp(Controls.Throttle, 0.0f, 1.0f);
    LastCausalActuation.EngineThrottlePreLimiter01 =
        Actuation.EngineThrottlePreLimiter01;
    LastCausalActuation.EngineThrottleFinal01 =
        Actuation.EngineThrottleFinal01;
    LastCausalActuation.EngineTorqueCurveNm = EngineTorqueCurveNm;
    LastCausalActuation.RequestedEngineTorqueAfterLimiterHealthNm =
        Actuation.RequestedEngineTorqueAfterLimiterHealthNm;
    LastCausalActuation.EffectiveGearRatio =
        Controls.EngagedGear != 0
            ? Movement->TransmissionSetup.GetGearRatio(Controls.EngagedGear)
            : 0.0f;
    LastCausalActuation.ConfiguredFinalDriveRatio =
        Movement->TransmissionSetup.FinalRatio;
    LastCausalActuation.TransmissionEfficiency =
        Movement->TransmissionSetup.TransmissionEfficiency;
    LastCausalActuation.ExternalRearDriveTorquePerWheelNm =
        Controls.ExternalRearDriveTorquePerWheelNm;
    LastCausalActuation.DriveTorquePath =
        FMath::Abs(Controls.ExternalRearDriveTorquePerWheelNm) > KINDA_SMALL_NUMBER
            ? EPinkCabCausalDriveTorquePath::ExternalPartialClutch
            : (Controls.EngagedGear != 0 && Controls.ClutchCoupling >= 1.0f
                ? EPinkCabCausalDriveTorquePath::ChaosMechanical
                : EPinkCabCausalDriveTorquePath::None);
    LastCausalActuation.bTorqueControlEnabled =
        Movement->TorqueControl.Enabled;
    LastCausalActuation.bTargetRotationControlEnabled =
        Movement->TargetRotationControl.Enabled;
    LastCausalActuation.bStabilizeControlEnabled =
        Movement->StabilizeControl.Enabled;
    LastCausalActuation.AssistContribution = 0.0f;

    Movement->SetThrottleInput(Actuation.EngineThrottleFinal01);
    Movement->SetBrakeInput(Controls.Brake);

    // PINK CAB owns a continuous parking/hydraulic handbrake actuator. Keep
    // Chaos' bool handbrake path disabled and add authored analog torque only
    // to the canonical rear axle.
    Movement->SetHandbrakeInput(false);
    constexpr float RearHandbrakeMaxTorqueNm = 1700.0f;
    const float RearBrakeTorqueNm =
        FMath::Clamp(Controls.Handbrake, 0.0f, 1.0f) * RearHandbrakeMaxTorqueNm;
    const int32 WheelCount = Movement->GetNumWheels();
    for (int32 WheelIndex = 2; WheelIndex < FMath::Min(WheelCount, 4); ++WheelIndex)
    {
        Movement->SetTorqueCombineMethod(
            ETorqueCombineMethod::Additive,
            WheelIndex);
        Movement->SetBrakeTorque(RearBrakeTorqueNm, WheelIndex);
        Movement->SetDriveTorque(
            Controls.ExternalRearDriveTorquePerWheelNm,
            WheelIndex);
    }

    LastCausalActuation.bAnyAbsConfigured = false;
    LastCausalActuation.bAnyTractionControlConfigured = false;
    for (UChaosVehicleWheel* Wheel : Movement->Wheels)
    {
        if (!Wheel)
        {
            continue;
        }
        LastCausalActuation.bAnyAbsConfigured |= Wheel->bABSEnabled;
        LastCausalActuation.bAnyTractionControlConfigured |=
            Wheel->bTractionControlEnabled;
    }
    return true;
}

bool FPinkCabChaosVehicleDynamicsProvider::ReadTelemetry(
    FPinkCabVehicleTelemetry& OutTelemetry) const
{
    if (!Movement)
    {
        return false;
    }

    OutTelemetry.SpeedKmh = Movement->GetForwardSpeed() * 0.036f;
    OutTelemetry.EngineRpm = Movement->GetEngineRotationSpeed();
    OutTelemetry.CurrentGear = Movement->GetCurrentGear();
    OutTelemetry.TargetGear = Movement->GetTargetGear();
    OutTelemetry.NormalizedSteering = LastControls.Steering;
    OutTelemetry.NormalizedThrottle = LastControls.Throttle;
    OutTelemetry.NormalizedBrake = LastControls.Brake;
    OutTelemetry.NormalizedClutch = LastControls.Clutch;
    OutTelemetry.NormalizedHandbrake = LastControls.Handbrake;
    OutTelemetry.CausalActuation = LastCausalActuation;

    OutTelemetry.Wheels.Reset();
    OutTelemetry.CausalWheels.Reset();
    const int32 WheelCount = Movement->GetNumWheels();
    OutTelemetry.Wheels.Reserve(WheelCount);
    OutTelemetry.CausalWheels.Reserve(WheelCount);
    for (int32 WheelIndex = 0; WheelIndex < WheelCount; ++WheelIndex)
    {
        const FWheelStatus Status = Movement->GetWheelState(WheelIndex);
        FPinkCabWheelTelemetry Wheel;
        Wheel.bInContact = Status.bInContact;
        Wheel.NormalizedSuspensionLength =
            Status.NormalizedSuspensionLength;
        Wheel.SpringForce = Status.SpringForce;
        Wheel.SlipAngle = Status.SlipAngle;
        Wheel.bIsSlipping = Status.bIsSlipping;
        Wheel.SlipMagnitude = Status.SlipMagnitude;
        Wheel.bIsSkidding = Status.bIsSkidding;
        Wheel.SkidMagnitude = Status.SkidMagnitude;
        Wheel.DriveTorque = Status.DriveTorque;
        Wheel.BrakeTorque = Status.BrakeTorque;
        Wheel.bABSActivated = Status.bABSActivated;
        OutTelemetry.Wheels.Add(Wheel);

        FPinkCabCausalWheelTelemetry CausalWheel;
        CausalWheel.WheelIndex = WheelIndex;
        CausalWheel.bInContact = Status.bInContact;
        CausalWheel.NormalizedSuspensionLength =
            Status.NormalizedSuspensionLength;
        CausalWheel.SuspensionSpringForce = Status.SpringForce;
        CausalWheel.SlipAngle = Status.SlipAngle;
        CausalWheel.SlipMagnitude = Status.SlipMagnitude;
        CausalWheel.DriveTorqueNm = Status.DriveTorque;
        CausalWheel.BrakeTorqueNm = Status.BrakeTorque;
        CausalWheel.bABSActivated = Status.bABSActivated;

        if (Movement->Wheels.IsValidIndex(WheelIndex))
        {
            if (const UChaosVehicleWheel* ChaosWheel =
                    Movement->Wheels[WheelIndex])
            {
                CausalWheel.WheelAngularVelocityRadPerSec =
                    ChaosWheel->GetWheelAngularVelocity();
                CausalWheel.WheelRpm =
                    CausalWheel.WheelAngularVelocityRadPerSec
                    * (60.0f / (2.0f * PI));
                CausalWheel.SteerAngleDeg =
                    ChaosWheel->GetSteerAngle();
                CausalWheel.bABSConfigured =
                    ChaosWheel->bABSEnabled;
                CausalWheel.bTractionControlConfigured =
                    ChaosWheel->bTractionControlEnabled;
            }
        }

        // UE 5.8 FWheelStatus does not expose exact normal load,
        // slip ratio, or longitudinal/lateral tire force. Availability
        // remains false instead of fabricating proxy values.
        OutTelemetry.CausalWheels.Add(CausalWheel);
    }
    return true;
}
