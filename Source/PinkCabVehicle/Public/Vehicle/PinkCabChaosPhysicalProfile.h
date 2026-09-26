#pragma once

#include "CoreMinimal.h"

class UChaosVehicleWheel;
class UChaosWheeledVehicleMovementComponent;

enum class EPinkCabPhysicalParameterAuthority : uint8
{
    Unspecified,
    Source,
    DesignTarget,
    Calibration
};

template <typename T>
struct TPinkCabPhysicalParameter
{
    T Value{};
    EPinkCabPhysicalParameterAuthority Authority = EPinkCabPhysicalParameterAuthority::Unspecified;
};

enum class EPinkCabCalibrationVariant : uint8
{
    Low,
    Nominal,
    High
};
struct FPinkCabChaosWheelPhysicalProfile
{
    TPinkCabPhysicalParameter<float> WheelRadiusCm;
    TPinkCabPhysicalParameter<float> WheelWidthCm;
    TPinkCabPhysicalParameter<float> WheelMassKg;
    TPinkCabPhysicalParameter<float> CorneringStiffness;
    TPinkCabPhysicalParameter<float> FrictionForceMultiplier;
    TPinkCabPhysicalParameter<float> SideSlipModifier;
    TPinkCabPhysicalParameter<float> SlipThreshold;
    TPinkCabPhysicalParameter<float> SkidThreshold;
    TPinkCabPhysicalParameter<float> MaxSteerAngleDeg;
    TPinkCabPhysicalParameter<float> MaxBrakeTorqueNm;
    TPinkCabPhysicalParameter<float> MaxHandBrakeTorqueNm;
    TPinkCabPhysicalParameter<float> SpringRate;
    TPinkCabPhysicalParameter<float> SpringPreload;
    TPinkCabPhysicalParameter<float> SuspensionMaxRaiseCm;
    TPinkCabPhysicalParameter<float> SuspensionMaxDropCm;
    TPinkCabPhysicalParameter<float> SuspensionDampingRatio;
    TPinkCabPhysicalParameter<float> WheelLoadRatio;
    TPinkCabPhysicalParameter<float> RollbarScaling;
    TPinkCabPhysicalParameter<bool> bABSEnabled;
    TPinkCabPhysicalParameter<bool> bTractionControlEnabled;
    TPinkCabPhysicalParameter<bool> bAffectedBySteering;
    TPinkCabPhysicalParameter<bool> bAffectedByEngine;
    TPinkCabPhysicalParameter<bool> bAffectedByBrake;
    TPinkCabPhysicalParameter<bool> bAffectedByHandbrake;
};
struct PINKCABVEHICLE_API FPinkCabChaosPhysicalProfile
{
    static FPinkCabChaosPhysicalProfile ForVariant(EPinkCabCalibrationVariant Variant);

    bool HasValidEnvelope() const;
    uint64 GetDeterministicProfileHash() const;
    bool HasCompleteProvenance() const;
    void ApplyToMovement(UChaosWheeledVehicleMovementComponent& Movement) const;
    void ApplyToFrontWheel(UChaosVehicleWheel& Wheel) const;
    void ApplyToRearWheel(UChaosVehicleWheel& Wheel) const;

    // P00-S versioned identity envelope. These fields identify the exact
    // calibration data without changing runtime driving behavior.
    FName ModelId;
    FName ProfileId;
    int32 SchemaVersion = 0;
    int32 CalibrationVersion = 0;
    FName UnitSystemId;
    FName ProvenanceSetId;
    FName CompatibilityId;
    FName MigrationId;
    EPinkCabCalibrationVariant CalibrationVariant = EPinkCabCalibrationVariant::Nominal;

    TPinkCabPhysicalParameter<float> ReferenceMassKg;
    TPinkCabPhysicalParameter<float> WheelbaseMm;
    TPinkCabPhysicalParameter<float> FrontTrackMm;
    TPinkCabPhysicalParameter<float> RearTrackMm;
    TPinkCabPhysicalParameter<float> MaxPowerHp;
    TPinkCabPhysicalParameter<float> MaxTorqueNm;
    TPinkCabPhysicalParameter<float> TerminalTargetKmh;
    TPinkCabPhysicalParameter<bool> bRearWheelDrive;

    TPinkCabPhysicalParameter<float> EngineMaxRpm;
    TPinkCabPhysicalParameter<float> EngineIdleRpm;
    TPinkCabPhysicalParameter<float> EngineBrakeEffect;
    TPinkCabPhysicalParameter<float> EngineRevUpMOI;
    TPinkCabPhysicalParameter<TArray<FVector2D>> NormalizedTorqueCurve;
    TPinkCabPhysicalParameter<bool> bUseAutomaticGears;
    TPinkCabPhysicalParameter<bool> bUseAutoReverse;
    TPinkCabPhysicalParameter<float> FinalDriveRatio;
    TPinkCabPhysicalParameter<TArray<float>> ForwardGearRatios;
    TPinkCabPhysicalParameter<TArray<float>> ReverseGearRatios;
    TPinkCabPhysicalParameter<float> SteeringAngleRatio;

    FPinkCabChaosWheelPhysicalProfile FrontWheel;
    FPinkCabChaosWheelPhysicalProfile RearWheel;
};
