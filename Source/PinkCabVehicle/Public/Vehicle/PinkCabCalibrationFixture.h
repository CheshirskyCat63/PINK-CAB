#pragma once

#include "CoreMinimal.h"

struct FPinkCabChaosPhysicalProfile;

struct PINKCABVEHICLE_API FPinkCabCalibrationInputKeyframe
{
    float TimeSeconds = 0.0f;
    float Throttle01 = 0.0f;
    float Brake01 = 0.0f;
    float Clutch01 = 0.0f;
    float Steering = 0.0f;
    int32 Gear = 0;
};

struct PINKCABVEHICLE_API FPinkCabCalibrationFixtureSpec
{
    FName FixtureId;
    int32 Seed = 0;
    float PhysicsDeltaSeconds = 1.0f / 120.0f;
    int32 FpsCap = 60;

    FName ModelId;
    FName ProfileId;
    int32 ProfileSchemaVersion = 0;
    int32 CalibrationVersion = 0;
    uint64 ProfileHash = 0;

    float VehicleMassKg = 0.0f;
    FName LoadStateId = TEXT("REFERENCE");
    FName SurfaceId = TEXT("DRY_ASPHALT");
    float SurfaceFrictionScale = 1.0f;
    FName TireStateId = TEXT("HEALTHY_COLD");
    float TireTemperatureC = 20.0f;
    float TireWear01 = 0.0f;

    TArray<FPinkCabCalibrationInputKeyframe> InputTrace;

    uint64 GetInputTraceHash() const;
    uint64 GetDeterministicHash() const;
    FString ToInputTraceCsv() const;
};

struct PINKCABVEHICLE_API FPinkCabCalibrationRunDescriptor
{
    FName FixtureId;
    int32 RunOrdinal = 0;
    int32 Seed = 0;
    float PhysicsDeltaSeconds = 0.0f;
    int32 FpsCap = 0;

    FName ModelId;
    FName ProfileId;
    int32 ProfileSchemaVersion = 0;
    int32 CalibrationVersion = 0;
    uint64 ProfileHash = 0;

    float VehicleMassKg = 0.0f;
    FName LoadStateId;
    FName SurfaceId;
    float SurfaceFrictionScale = 0.0f;
    FName TireStateId;
    float TireTemperatureC = 0.0f;
    float TireWear01 = 0.0f;

    uint64 InputTraceHash = 0;
    uint64 FixtureHash = 0;

    static FPinkCabCalibrationRunDescriptor FromFixture(
        const FPinkCabCalibrationFixtureSpec& Fixture,
        int32 RunOrdinal);

    bool IsComparableTo(const FPinkCabCalibrationRunDescriptor& Other) const;
    FString ToMetadataText() const;
};

struct PINKCABVEHICLE_API FPinkCabCalibrationFixtureLibrary
{
    static TArray<FPinkCabCalibrationFixtureSpec> BuildCanonicalSet(
        const FPinkCabChaosPhysicalProfile& Profile);
};
