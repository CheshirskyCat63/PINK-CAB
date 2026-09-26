#include "Runtime/PinkCabChaosTatraPawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Vehicle/PinkCabCausalTelemetryFrameBuilder.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"

namespace
{
EPinkCabCausalIgnitionState ToCausalIgnitionState(
    const EPinkCabIgnitionState State)
{
    switch (State)
    {
    case EPinkCabIgnitionState::Running:
        return EPinkCabCausalIgnitionState::Running;
    case EPinkCabIgnitionState::Stalled:
        return EPinkCabCausalIgnitionState::Stalled;
    default:
        return EPinkCabCausalIgnitionState::Off;
    }
}
}

void APinkCabChaosTatraPawn::RecordCausalTelemetry(
    const double NowSeconds,
    const float DeltaSeconds)
{
    if (!bCausalTelemetryEnabled || CausalTelemetryTrace.IsSaturated())
    {
        return;
    }

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    if (!Movement)
    {
        return;
    }

    FPinkCabVehicleTelemetry VehicleTelemetry;
    if (!DynamicsProvider.ReadTelemetry(VehicleTelemetry))
    {
        return;
    }

    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(
            EPinkCabCalibrationVariant::Nominal);

    FPinkCabCausalTelemetryBuildInput Input;
    Input.TimestampSeconds = NowSeconds;
    Input.DeltaSeconds = DeltaSeconds;
    Input.IgnitionState =
        ToCausalIgnitionState(CockpitState.GetIgnitionState());
    Input.ModelId = Profile.ModelId;
    Input.ProfileId = Profile.ProfileId;
    Input.ProfileSchemaVersion = Profile.SchemaVersion;
    Input.CalibrationVersion = Profile.CalibrationVersion;
    Input.ProfileHash = Profile.GetDeterministicProfileHash();
    Input.Control = VehicleControlRuntime.GetCausalControlTelemetry();
    Input.Controls = VehicleControlRuntime.GetControlState();
    Input.Vehicle = MoveTemp(VehicleTelemetry);
    Input.EngineHealthFactor01 =
        VehicleHealthService.GetEnginePowerEffectiveness01(
            GetVehicleHealthState());
    Input.WorldSpeedMps = GetVelocity().Size() * 0.01f;
    Input.VehicleMassKg = Movement->Mass;

    CausalTelemetryTrace.Record(
        FPinkCabCausalTelemetryFrameBuilder::Build(Input));
}

void APinkCabChaosTatraPawn::FlushCausalTelemetry()
{
    if (!bCausalTelemetryEnabled)
    {
        return;
    }

    const FString Directory =
        FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GitHubGate"));
    IFileManager::Get().MakeDirectory(*Directory, true);

    const FString FramePath =
        FPaths::Combine(Directory, TEXT("VEHICLE_PHYSICS_CAUSAL_TRACE.csv"));
    const FString WheelPath =
        FPaths::Combine(Directory, TEXT("VEHICLE_PHYSICS_CAUSAL_WHEELS.csv"));
    const FString MetadataPath =
        FPaths::Combine(Directory, TEXT("VEHICLE_PHYSICS_CAUSAL_TRACE.txt"));

    const bool bFramesSaved = FFileHelper::SaveStringToFile(
        CausalTelemetryTrace.ToCsv(),
        *FramePath,
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const bool bWheelsSaved = FFileHelper::SaveStringToFile(
        CausalTelemetryTrace.ToWheelCsv(),
        *WheelPath,
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const FString Metadata = FString::Printf(
        TEXT("frames=%d\ncapacity=%d\ndropped=%d\nsaturated=%d\n"),
        CausalTelemetryTrace.GetNum(),
        CausalTelemetryTrace.GetCapacity(),
        CausalTelemetryTrace.GetDroppedFrameCount(),
        CausalTelemetryTrace.IsSaturated() ? 1 : 0);
    const bool bMetadataSaved = FFileHelper::SaveStringToFile(
        Metadata,
        *MetadataPath,
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

    UE_LOG(
        LogTemp,
        Display,
        TEXT("PINKCAB_PHY003_TRACE_FLUSH frames=%d dropped=%d frames_saved=%d wheels_saved=%d metadata_saved=%d"),
        CausalTelemetryTrace.GetNum(),
        CausalTelemetryTrace.GetDroppedFrameCount(),
        bFramesSaved ? 1 : 0,
        bWheelsSaved ? 1 : 0,
        bMetadataSaved ? 1 : 0);
}
