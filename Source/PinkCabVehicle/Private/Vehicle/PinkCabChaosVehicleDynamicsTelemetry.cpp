#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

namespace
{
FPinkCabWheelTelemetry MakeLegacyWheelTelemetry(
    const FWheelStatus& Status)
{
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
    return Wheel;
}

FPinkCabCausalWheelTelemetry MakeCausalWheelTelemetry(
    const UChaosWheeledVehicleMovementComponent& Movement,
    const int32 WheelIndex,
    const FWheelStatus& Status)
{
    FPinkCabCausalWheelTelemetry Wheel;
    Wheel.WheelIndex = WheelIndex;
    Wheel.bInContact = Status.bInContact;
    Wheel.NormalizedSuspensionLength =
        Status.NormalizedSuspensionLength;
    Wheel.SuspensionSpringForce = Status.SpringForce;
    Wheel.SlipAngle = Status.SlipAngle;
    Wheel.SlipMagnitude = Status.SlipMagnitude;
    Wheel.DriveTorqueNm = Status.DriveTorque;
    Wheel.BrakeTorqueNm = Status.BrakeTorque;
    Wheel.bABSActivated = Status.bABSActivated;

    if (Movement.Wheels.IsValidIndex(WheelIndex))
    {
        if (const UChaosVehicleWheel* ChaosWheel =
                Movement.Wheels[WheelIndex])
        {
            Wheel.WheelAngularVelocityRadPerSec =
                ChaosWheel->GetWheelAngularVelocity();
            Wheel.WheelRpm =
                Wheel.WheelAngularVelocityRadPerSec
                * (60.0f / (2.0f * PI));
            Wheel.SteerAngleDeg = ChaosWheel->GetSteerAngle();
            Wheel.bABSConfigured = ChaosWheel->bABSEnabled;
            Wheel.bTractionControlConfigured =
                ChaosWheel->bTractionControlEnabled;
            Wheel.bEngineDriven = ChaosWheel->bAffectedByEngine;
        }
    }

    // UE 5.8 FWheelStatus does not expose exact normal load, slip ratio,
    // or longitudinal/lateral tire force. Availability remains false.
    return Wheel;
}
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
    OutTelemetry.NativeCurrentGear = Movement->GetCurrentGear();
    OutTelemetry.NativeTargetGear = Movement->GetTargetGear();
    OutTelemetry.CurrentGear = OutTelemetry.NativeCurrentGear;
    OutTelemetry.TargetGear = OutTelemetry.NativeTargetGear;
    if (const auto* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement))
    {
        // Read the movement-owned command, not this provider handle's cache:
        // fixture and observer handles can differ from the last command writer.
        const auto& Command = PinkCabMovement->GetPendingPinkCabDrivelineCommand();
        OutTelemetry.CurrentGear = Command.EngagedGear;
        OutTelemetry.TargetGear = Command.RequestedGear;
    }
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
        OutTelemetry.Wheels.Add(MakeLegacyWheelTelemetry(Status));
        OutTelemetry.CausalWheels.Add(
            MakeCausalWheelTelemetry(*Movement, WheelIndex, Status));
    }
    return true;
}
