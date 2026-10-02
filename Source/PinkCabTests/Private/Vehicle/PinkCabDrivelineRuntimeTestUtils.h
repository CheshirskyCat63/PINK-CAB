#pragma once

#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Vehicle/PinkCabPhysicsFixturePawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"

namespace PinkCabDrivelineRuntimeTest
{
inline bool Apply(
    APinkCabPhysicsFixturePawn& Pawn,
    FPinkCabCockpitState& Cockpit,
    FPinkCabVehicleControlState& Controls)
{
    UChaosWheeledVehicleMovementComponent* Movement = Pawn.GetChaosMovement();
    if (!Movement)
    {
        return false;
    }
    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
    return FPinkCabChaosCockpitBridge::Apply(
        Cockpit, *Movement, Controls, Provider);
}

inline bool AdvanceMechanicalTime(
    UPinkCabChaosVehicleMovementComponent& Movement,
    int64& LastStep,
    double& ElapsedSeconds)
{
    const int64 CurrentStep =
        Movement.GetPinkCabMechanicalIntegrationStepCount();
    if (LastStep < 0)
    {
        LastStep = CurrentStep;
        return false;
    }
    if (CurrentStep <= LastStep)
    {
        return false;
    }

    const int64 StepDelta = CurrentStep - LastStep;
    const float DeltaSeconds =
        Movement.GetPinkCabLastMechanicalIntegrationDeltaSeconds();
    LastStep = CurrentStep;
    if (DeltaSeconds <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    ElapsedSeconds +=
        static_cast<double>(StepDelta)
        * static_cast<double>(DeltaSeconds);
    return true;
}

inline float HorizontalSpeedCmPerSec(USkeletalMeshComponent& Mesh)
{
    const FVector Velocity = Mesh.GetPhysicsLinearVelocity();
    return FVector2D(Velocity.X, Velocity.Y).Size();
}

inline float SignedForwardDistanceCm(
    const FVector& Start,
    const FVector& Current,
    const FVector& Forward)
{
    return FVector::DotProduct(Current - Start, Forward);
}

inline float MeanRearDriveTorqueNm(
    UChaosWheeledVehicleMovementComponent& Movement)
{
    const FWheelStatus RearLeft = Movement.GetWheelState(2);
    const FWheelStatus RearRight = Movement.GetWheelState(3);
    return 0.5f * (RearLeft.DriveTorque + RearRight.DriveTorque);
}

inline FPinkCabVehicleTelemetry BuildTelemetry(
    UChaosWheeledVehicleMovementComponent& Movement,
    USkeletalMeshComponent& Mesh)
{
    FPinkCabVehicleTelemetry Telemetry;
    FPinkCabChaosVehicleDynamicsProvider Provider(&Movement);
    Provider.ReadTelemetry(Telemetry);
    Telemetry.SpeedKmh = HorizontalSpeedCmPerSec(Mesh) * 0.036f;
    Telemetry.NormalizedThrottle = Movement.GetThrottleInput();
    Telemetry.NormalizedBrake = Movement.GetBrakeInput();
    return Telemetry;
}
}
