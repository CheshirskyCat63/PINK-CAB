#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"

#include "ChaosWheeledVehicleMovementComponent.h"

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
    Movement->SetSteeringInput(-Controls.Steering);
    Movement->SetThrottleInput(Controls.Throttle);
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
