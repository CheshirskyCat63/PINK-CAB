#pragma once

#include "CoreMinimal.h"

enum class EPinkCabCausalIgnitionState : uint8
{
    Off,
    Running,
    Stalled
};

enum class EPinkCabCausalDriveTorquePath : uint8
{
    None,
    ExternalPartialClutch,
    ChaosMechanical
};

struct PINKCABVEHICLE_API FPinkCabCausalControlTelemetry
{
    float RawThrottle01 = 0.0f;
    float RawBrake01 = 0.0f;
    float RawClutch01 = 0.0f;
    float PreparedThrottle01 = 0.0f;
    float PreparedBrake01 = 0.0f;
    float PreparedClutch01 = 0.0f;
    float PostDrivetrainThrottle01 = 0.0f;
    float RawSteeringMouseDelta = 0.0f;
    float SteeringVirtualCursor = 0.0f;
    float SteeringTarget = 0.0f;
    float FinalSteeringCommand = 0.0f;
    float ExpectedCoupledRpm = 0.0f;
    float EngineTorqueFactor = 1.0f;
    float BrakeEffectiveness = 1.0f;
    float DrivetrainTorqueCapacity01 = 1.0f;
};

struct PINKCABVEHICLE_API FPinkCabCausalActuationTelemetry
{
    float HealthClampedControlThrottle01 = 0.0f;
    float EngineThrottlePreLimiter01 = 0.0f;
    float EngineThrottleFinal01 = 0.0f;
    float EngineTorqueCurveNm = 0.0f;
    float RequestedEngineTorqueAfterLimiterHealthNm = 0.0f;
    float EffectiveGearRatio = 0.0f;
    float ConfiguredFinalDriveRatio = 0.0f;
    float TransmissionEfficiency = 0.0f;
    float ExternalRearDriveTorquePerWheelNm = 0.0f;
    EPinkCabCausalDriveTorquePath DriveTorquePath = EPinkCabCausalDriveTorquePath::None;
    bool bTorqueControlEnabled = false;
    bool bTargetRotationControlEnabled = false;
    bool bStabilizeControlEnabled = false;
    bool bAnyAbsConfigured = false;
    bool bAnyTractionControlConfigured = false;
    float AssistContribution = 0.0f;
};

struct PINKCABVEHICLE_API FPinkCabCausalWheelTelemetry
{
    int32 WheelIndex = INDEX_NONE;
    bool bInContact = false;
    float WheelAngularVelocityRadPerSec = 0.0f;
    float WheelRpm = 0.0f;
    float SteerAngleDeg = 0.0f;
    float NormalizedSuspensionLength = 1.0f;
    float SuspensionSpringForce = 0.0f;
    float SlipAngle = 0.0f;
    float SlipMagnitude = 0.0f;
    float DriveTorqueNm = 0.0f;
    float BrakeTorqueNm = 0.0f;
    bool bABSConfigured = false;
    bool bABSActivated = false;
    bool bTractionControlConfigured = false;

    // UE 5.8 public FWheelStatus does not expose exact tire normal load,
    // slip ratio, or longitudinal/lateral tire force. Keep absence explicit.
    bool bHasNormalLoad = false;
    float NormalLoadN = 0.0f;
    bool bHasSlipRatio = false;
    float SlipRatio = 0.0f;
    bool bHasLongitudinalForce = false;
    float LongitudinalForceN = 0.0f;
    bool bHasLateralForce = false;
    float LateralForceN = 0.0f;
};

struct PINKCABVEHICLE_API FPinkCabCausalTelemetryFrame
{
    uint64 Sequence = 0;
    double TimestampSeconds = 0.0;
    float DeltaSeconds = 0.0f;

    EPinkCabCausalIgnitionState IgnitionState = EPinkCabCausalIgnitionState::Off;
    bool bCombustionPermission = false;

    FName ModelId;
    FName ProfileId;
    int32 ProfileSchemaVersion = 0;
    int32 CalibrationVersion = 0;
    uint64 ProfileHash = 0;

    FPinkCabCausalControlTelemetry Control;
    FPinkCabCausalActuationTelemetry Actuation;

    float EngineRpm = 0.0f;
    float ExpectedCoupledRpm = 0.0f;
    float ClutchCoupling01 = 0.0f;
    float ClutchSlipRpm = 0.0f;
    float DrivetrainTorqueCapacity01 = 1.0f;
    float EngineHealthFactor01 = 1.0f;
    float RequestedEngineTorqueAfterLimiterHealthNm = 0.0f;
    float PermissionGatedAvailableEngineTorqueNm = 0.0f;

    int32 RequestedGear = 0;
    int32 EngagedGear = 0;
    int32 ChaosCurrentGear = 0;
    int32 ChaosTargetGear = 0;

    float ForwardSpeedKmh = 0.0f;
    float WorldSpeedMps = 0.0f;
    float VehicleMassKg = 0.0f;
    double TranslationalKineticEnergyJ = 0.0;

    TArray<FPinkCabCausalWheelTelemetry> Wheels;

    void RefreshDerivedFields();
};

class PINKCABVEHICLE_API FPinkCabCausalTelemetryTrace
{
public:
    explicit FPinkCabCausalTelemetryTrace(int32 InCapacity = 4096);

    void Reset();
    bool Record(FPinkCabCausalTelemetryFrame Frame);

    int32 GetNum() const { return Frames.Num(); }
    int32 GetCapacity() const { return Capacity; }
    int32 GetDroppedFrameCount() const { return DroppedFrameCount; }
    bool IsSaturated() const { return Frames.Num() >= Capacity; }
    const TArray<FPinkCabCausalTelemetryFrame>& GetFrames() const { return Frames; }

    FString ToCsv() const;

private:
    int32 Capacity = 4096;
    int32 DroppedFrameCount = 0;
    uint64 NextSequence = 0;
    TArray<FPinkCabCausalTelemetryFrame> Frames;
};
