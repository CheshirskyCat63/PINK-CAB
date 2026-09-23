#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabThrottleResponse.h"

#include "ChaosWheeledVehicleMovementComponent.h"

namespace
{
float ApplyPinkCabRevLimiter(float RequestedEngineThrottle, float EngineRpm, float MaxRpm)
{
    if (MaxRpm <= KINDA_SMALL_NUMBER)
    {
        return RequestedEngineThrottle;
    }

    // UE 5.8 Chaos does not clamp free-running engine Omega to MaxRPM. Once a
    // large physics step overshoots MaxRPM it can keep producing clamped-curve
    // torque and run away. PINK CAB owns the limiter at the control boundary so
    // we do not patch Engine source and the behavior remains deterministic.
    constexpr float SoftLimiterStartRatio = 0.965f;
    constexpr float HardLimiterCutRatio = 0.995f;
    const float SoftStartRpm = MaxRpm * SoftLimiterStartRatio;
    const float HardCutRpm = MaxRpm * HardLimiterCutRatio;
    if (EngineRpm <= SoftStartRpm)
    {
        return RequestedEngineThrottle;
    }
    if (EngineRpm >= HardCutRpm)
    {
        return 0.0f;
    }

    const float Remaining = 1.0f - FMath::Clamp(
        (EngineRpm - SoftStartRpm) / (HardCutRpm - SoftStartRpm), 0.0f, 1.0f);
    return RequestedEngineThrottle * Remaining;
}
}

FPinkCabChaosVehicleDynamicsProvider::FPinkCabChaosVehicleDynamicsProvider(
    UChaosWheeledVehicleMovementComponent* InMovement)
    : Movement(InMovement)
{
}

bool FPinkCabChaosVehicleDynamicsProvider::ApplyControls(const FPinkCabVehicleControlState& Controls)
{
    if (!Movement)
    {
        return false;
    }

    LastControls = Controls;
    // PINK CAB semantic steering is permanently player-facing: + = right.
    // Runtime signed-displacement evidence on the Tatra chassis confirms that
    // Chaos expects the same sign here. Do not add another inversion at this
    // boundary; input/controller and physical vehicle must stay right-positive.
    Movement->SetSteeringInput(Controls.Steering);
    const float RequestedEngineThrottle =
        FPinkCabThrottleResponse::ToEngineThrottle(Controls.Throttle);
    const float LimitedEngineThrottle = ApplyPinkCabRevLimiter(
        RequestedEngineThrottle,
        Movement->GetEngineRotationSpeed(),
        Movement->EngineSetup.MaxRPM);
    Movement->SetThrottleInput(LimitedEngineThrottle);
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
        Movement->SetTorqueCombineMethod(ETorqueCombineMethod::Additive, WheelIndex);
        Movement->SetBrakeTorque(RearBrakeTorqueNm, WheelIndex);
        Movement->SetDriveTorque(Controls.ExternalRearDriveTorquePerWheelNm, WheelIndex);
    }
    return true;
}

bool FPinkCabChaosVehicleDynamicsProvider::ReadTelemetry(FPinkCabVehicleTelemetry& OutTelemetry) const
{
    if (!Movement)
    {
        return false;
    }

    OutTelemetry.SpeedKmh = Movement->GetForwardSpeed() * 0.036f;
    OutTelemetry.EngineRpm = Movement->GetEngineRotationSpeed();
    OutTelemetry.CurrentGear = Movement->GetCurrentGear();
    OutTelemetry.NormalizedSteering = LastControls.Steering;
    OutTelemetry.NormalizedThrottle = LastControls.Throttle;
    OutTelemetry.NormalizedBrake = LastControls.Brake;
    OutTelemetry.NormalizedClutch = LastControls.Clutch;
    OutTelemetry.NormalizedHandbrake = LastControls.Handbrake;

    OutTelemetry.Wheels.Reset();
    const int32 WheelCount = Movement->GetNumWheels();
    OutTelemetry.Wheels.Reserve(WheelCount);
    for (int32 WheelIndex = 0; WheelIndex < WheelCount; ++WheelIndex)
    {
        const FWheelStatus Status = Movement->GetWheelState(WheelIndex);
        FPinkCabWheelTelemetry Wheel;
        Wheel.bInContact = Status.bInContact;
        Wheel.NormalizedSuspensionLength = Status.NormalizedSuspensionLength;
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
    }
    return true;
}
