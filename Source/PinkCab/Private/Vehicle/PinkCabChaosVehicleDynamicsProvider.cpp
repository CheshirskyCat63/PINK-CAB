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
    Movement->SetSteeringInput(Controls.Steering);
    Movement->SetThrottleInput(Controls.Throttle);
    Movement->SetBrakeInput(Controls.Brake);
    Movement->SetHandbrakeInput(Controls.Handbrake >= 0.5f);
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
    OutTelemetry.NormalizedSteering = LastControls.Steering;
    OutTelemetry.NormalizedThrottle = LastControls.Throttle;
    OutTelemetry.NormalizedBrake = LastControls.Brake;
    OutTelemetry.NormalizedClutch = LastControls.Clutch;
    OutTelemetry.NormalizedHandbrake = LastControls.Handbrake;
    return true;
}
