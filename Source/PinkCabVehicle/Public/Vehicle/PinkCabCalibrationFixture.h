#pragma once

#include "CoreMinimal.h"

enum class EPinkCabCalibrationFixtureKind : uint8
{
    FlatLaunch,
    GradeLaunch,
    LimiterCouplingSweep,
    ConstantRadius,
    CombinedBrakeTurn,
    CombinedThrottleTurn,
    LiftOff,
    SplitMu,
    Braking,
    Slalom,
    RoughRoad
};

struct PINKCABVEHICLE_API FPinkCabCalibrationInputSample
{
    float Throttle01 = 0.0f;
    float Brake01 = 0.0f;
    float Clutch01 = 0.0f;
    float Handbrake01 = 0.0f;
    float Steering = 0.0f;
    int32 RequestedGear = 0;
};

struct PINKCABVEHICLE_API FPinkCabCalibrationInputKeyframe
    : public FPinkCabCalibrationInputSample
{
    double TimeSeconds = 0.0;
};

struct PINKCABVEHICLE_API FPinkCabCalibrationFixture
{
    FName FixtureId;
    EPinkCabCalibrationFixtureKind Kind =
        EPinkCabCalibrationFixtureKind::FlatLaunch;
    FString MapPath;
    FName EnvironmentId;
    FName SurfaceId;
    FName LoadFixtureId;
    FName ModelId;
    FName ProfileId;
    int32 ProfileSchemaVersion = 0;
    int32 CalibrationVersion = 0;
    uint64 ProfileHash = 0;
    float LoadMassKg = 0.0f;
    int32 FpsCap = 60;
    double FixedDeltaSeconds = 1.0 / 60.0;
    TArray<FPinkCabCalibrationInputKeyframe> InputTrace;

    bool IsValid() const;
    uint64 GetDeterministicHash() const;
    FPinkCabCalibrationInputSample SampleAtSeconds(double TimeSeconds) const;
    FString ToCsv() const;
};

struct PINKCABVEHICLE_API FPinkCabCalibrationFixtureCatalog
{
    static FPinkCabCalibrationFixture Build(
        EPinkCabCalibrationFixtureKind Kind,
        int32 FpsCap);
    static TArray<FPinkCabCalibrationFixture> BuildCanonicalMatrix();
};
