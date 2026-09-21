#include "Runtime/PinkCabChaosTatraPawn.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"

namespace
{
struct FPinkCabPackagedGateChaosState
{
    int32 WheelCount = 0;
    int32 ContactCount = 0;
    int32 CurrentGear = 0;
    int32 TargetGear = 0;
    float EngineRpm = 0.0f;
    float RearLeftDriveTorque = 0.0f;
    float RearRightDriveTorque = 0.0f;
    float RearLeftBrakeTorque = 0.0f;
    float RearRightBrakeTorque = 0.0f;
};

FPinkCabPackagedGateChaosState CapturePackagedGateChaosState(
    const UChaosWheeledVehicleMovementComponent* Movement)
{
    FPinkCabPackagedGateChaosState State;
    if (!Movement)
    {
        return State;
    }

    State.WheelCount = Movement->GetNumWheels();
    State.CurrentGear = Movement->GetCurrentGear();
    State.TargetGear = Movement->GetTargetGear();
    State.EngineRpm = Movement->GetEngineRotationSpeed();
    for (int32 WheelIndex = 0; WheelIndex < State.WheelCount; ++WheelIndex)
    {
        State.ContactCount += Movement->GetWheelState(WheelIndex).bInContact ? 1 : 0;
    }

    if (State.WheelCount > 2)
    {
        const FWheelStatus RearLeft = Movement->GetWheelState(2);
        State.RearLeftDriveTorque = RearLeft.DriveTorque;
        State.RearLeftBrakeTorque = RearLeft.BrakeTorque;
    }
    if (State.WheelCount > 3)
    {
        const FWheelStatus RearRight = Movement->GetWheelState(3);
        State.RearRightDriveTorque = RearRight.DriveTorque;
        State.RearRightBrakeTorque = RearRight.BrakeTorque;
    }
    return State;
}
}

void APinkCabChaosTatraPawn::EmitPackagedGateTelemetry(const double NowSeconds)
{
    if (!bPackagedGateTelemetryEnabled || NowSeconds < NextPackagedGateTelemetrySeconds)
    {
        return;
    }
    NextPackagedGateTelemetrySeconds = NowSeconds + 0.10;

    FPinkCabVehicleTelemetry Telemetry{};
    DynamicsProvider.ReadTelemetry(Telemetry);
    const float DistanceCm = FVector::Dist2D(GetActorLocation(), PackagedGateStartLocation);
    const FVector2D Cursor = GetGearLeverVisualCursor();

    FName TargetId = NAME_None;
    bool bGrip = false;
    bool bManipulation = false;
    if (CockpitInteraction)
    {
        TargetId = CockpitInteraction->GetCurrentTargetId();
        bGrip = CockpitInteraction->IsGripActive();
        bManipulation = CockpitInteraction->IsManipulationActive();
    }

    const FPinkCabPackagedGateChaosState Chaos =
        CapturePackagedGateChaosState(GetChaosMovement());
    const bool bIgnitionRunning =
        CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    const bool bDriverCameraActive =
        DriverCamera && DriverCamera->IsActive();

    UE_LOG(
        LogTemp,
        Display,
        TEXT("PINKCAB_GATE_STATE menu=%d ignition=%d requested=%d engaged=%d throttle=%.3f brake=%.3f clutch=%.3f handbrake=%.3f steering=%.3f speed=%.3f dist=%.1f gearx=%.3f geary=%.3f target=%s grip=%d manipulation=%d camera=%d wheels=%d contacts=%d chaos_current=%d chaos_target=%d rpm=%.1f rear_drive=(%.1f,%.1f) rear_brake=(%.1f,%.1f)"),
        static_cast<int32>(IsSystemMenuOpen()),
        static_cast<int32>(bIgnitionRunning),
        GetRequestedGear(),
        GetEngagedGear(),
        Telemetry.NormalizedThrottle,
        Telemetry.NormalizedBrake,
        Telemetry.NormalizedClutch,
        Telemetry.NormalizedHandbrake,
        Telemetry.NormalizedSteering,
        Telemetry.SpeedKmh,
        DistanceCm,
        Cursor.X,
        Cursor.Y,
        *TargetId.ToString(),
        static_cast<int32>(bGrip),
        static_cast<int32>(bManipulation),
        static_cast<int32>(bDriverCameraActive),
        Chaos.WheelCount,
        Chaos.ContactCount,
        Chaos.CurrentGear,
        Chaos.TargetGear,
        Chaos.EngineRpm,
        Chaos.RearLeftDriveTorque,
        Chaos.RearRightDriveTorque,
        Chaos.RearLeftBrakeTorque,
        Chaos.RearRightBrakeTorque);
}
