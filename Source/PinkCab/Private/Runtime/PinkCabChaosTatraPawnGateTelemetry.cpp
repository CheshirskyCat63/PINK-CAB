#include "Runtime/PinkCabChaosTatraPawn.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

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


struct FPinkCabPackagedGateAimState
{
    bool bValid = false;
    float IgnitionYawDeg = 0.0f;
    float IgnitionPitchDeg = 0.0f;
};

FPinkCabPackagedGateAimState CapturePackagedGateAimState(
    const UCameraComponent* Camera,
    const UPinkCabCockpitAssemblyComponent* Assembly)
{
    FPinkCabPackagedGateAimState State;
    if (!Camera || !Assembly)
    {
        return State;
    }
    const USceneComponent* Ignition =
        Assembly->GetSlotComponent(EPinkCabCockpitSlot::Ignition);
    if (!Ignition)
    {
        return State;
    }

    const FVector ToIgnitionWorld =
        Ignition->GetComponentLocation() - Camera->GetComponentLocation();
    const FVector Local =
        Camera->GetComponentTransform().InverseTransformVectorNoScale(ToIgnitionWorld);
    const float Horizontal = FMath::Sqrt(Local.X * Local.X + Local.Y * Local.Y);
    if (Local.IsNearlyZero() || Horizontal <= KINDA_SMALL_NUMBER)
    {
        return State;
    }

    State.bValid = true;
    State.IgnitionYawDeg =
        FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));
    State.IgnitionPitchDeg =
        FMath::RadiansToDegrees(FMath::Atan2(Local.Z, Horizontal));
    return State;
}

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
    const FVector GateDisplacement = GetActorLocation() - PackagedGateStartLocation;
    const float DistanceCm = FVector::Dist2D(GetActorLocation(), PackagedGateStartLocation);
    const float LongitudinalCm = FVector::DotProduct(GateDisplacement, PackagedGateStartForward);
    const FVector2D Cursor = GetGearLeverVisualCursor();

    FName TargetId = NAME_None;
    bool bGrip = false;
    bool bManipulation = false;
    bool bGazeHeld = false;
    if (CockpitInteraction)
    {
        TargetId = CockpitInteraction->GetCurrentTargetId();
        bGrip = CockpitInteraction->IsGripActive();
        bManipulation = CockpitInteraction->IsManipulationActive();
        bGazeHeld = CockpitInteraction->IsGazeHeld();
    }

    const FPinkCabPackagedGateChaosState Chaos =
        CapturePackagedGateChaosState(GetChaosMovement());
    const FPinkCabPackagedGateAimState Aim =
        CapturePackagedGateAimState(DriverCamera, CockpitAssembly);
    const bool bIgnitionRunning =
        CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    const bool bDriverCameraActive =
        DriverCamera && DriverCamera->IsActive();
    const bool bWorldPaused = UGameplayStatics::IsGamePaused(this);
    const float WorldDeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : -1.0f;
    const float ActorVelocityCmPerSec = GetVelocity().Size2D();
    const APlayerController* PC = Cast<APlayerController>(GetController());
    const bool bQHeld = PC && PC->IsInputKeyDown(EKeys::Q);
    const bool bWHeld = PC && PC->IsInputKeyDown(EKeys::W);
    const bool bEHeld = PC && PC->IsInputKeyDown(EKeys::E);
    const int32 WheelRecipient = static_cast<int32>(VehicleControlRuntime.GetLastWheelRecipient());
    const float ThrottleTarget = VehicleControlRuntime.GetThrottleTarget();
    const uint32 LaunchSerial = VehicleControlRuntime.GetLaunchSerial();
    USkeletalMeshComponent* VehicleMesh = GetMesh();
    const bool bAnyRigidBodyAwake =
        VehicleMesh && VehicleMesh->IsAnyRigidBodyAwake();

    UE_LOG(
        LogTemp,
        Display,
        TEXT("PINKCAB_GATE_STATE menu=%d paused=%d worlddt=%.4f awake=%d velcm=%.3f ignition=%d requested=%d engaged=%d throttle=%.3f brake=%.3f clutch=%.3f handbrake=%.3f steering=%.3f speed=%.3f dist=%.1f longcm=%.1f gearx=%.3f geary=%.3f target=%s grip=%d manipulation=%d gaze=%d camera=%d aimvalid=%d aimyaw=%.2f aimpitch=%.2f wheels=%d qheld=%d wheld=%d eheld=%d wheelpending=%d wheelrecipient=%d throttletarget=%.3f launchserial=%u contacts=%d chaos_current=%d chaos_target=%d rpm=%.1f rear_drive=(%.1f,%.1f) rear_brake=(%.1f,%.1f)"),
        static_cast<int32>(IsSystemMenuOpen()),
        static_cast<int32>(bWorldPaused),
        WorldDeltaSeconds,
        static_cast<int32>(bAnyRigidBodyAwake),
        ActorVelocityCmPerSec,
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
        LongitudinalCm,
        Cursor.X,
        Cursor.Y,
        *TargetId.ToString(),
        static_cast<int32>(bGrip),
        static_cast<int32>(bManipulation),
        static_cast<int32>(bGazeHeld),
        static_cast<int32>(bDriverCameraActive),
        static_cast<int32>(Aim.bValid),
        Aim.IgnitionYawDeg,
        Aim.IgnitionPitchDeg,
        Chaos.WheelCount,
        static_cast<int32>(bQHeld),
        static_cast<int32>(bWHeld),
        static_cast<int32>(bEHeld),
        PendingMouseWheelSteps,
        WheelRecipient,
        ThrottleTarget,
        LaunchSerial,
        Chaos.ContactCount,
        Chaos.CurrentGear,
        Chaos.TargetGear,
        Chaos.EngineRpm,
        Chaos.RearLeftDriveTorque,
        Chaos.RearRightDriveTorque,
        Chaos.RearLeftBrakeTorque,
        Chaos.RearRightBrakeTorque);
}
