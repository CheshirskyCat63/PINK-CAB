#include "Vehicle/PinkCabCalibrationFixture.h"

#include "Vehicle/PinkCabChaosPhysicalProfile.h"

namespace
{
constexpr const TCHAR* CanonicalMap = TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave");

const TCHAR* KindToken(const EPinkCabCalibrationFixtureKind Kind)
{
    switch (Kind)
    {
    case EPinkCabCalibrationFixtureKind::GradeLaunch: return TEXT("GRADE_LAUNCH");
    case EPinkCabCalibrationFixtureKind::LimiterCouplingSweep: return TEXT("LIMITER_COUPLING");
    case EPinkCabCalibrationFixtureKind::ConstantRadius: return TEXT("CONSTANT_RADIUS");
    case EPinkCabCalibrationFixtureKind::CombinedBrakeTurn: return TEXT("BRAKE_TURN");
    case EPinkCabCalibrationFixtureKind::CombinedThrottleTurn: return TEXT("THROTTLE_TURN");
    case EPinkCabCalibrationFixtureKind::LiftOff: return TEXT("LIFT_OFF");
    case EPinkCabCalibrationFixtureKind::SplitMu: return TEXT("SPLIT_MU");
    case EPinkCabCalibrationFixtureKind::Braking: return TEXT("BRAKING");
    case EPinkCabCalibrationFixtureKind::Slalom: return TEXT("SLALOM");
    case EPinkCabCalibrationFixtureKind::RoughRoad: return TEXT("ROUGH_ROAD");
    default: return TEXT("FLAT_LAUNCH");
    }
}

FName SurfaceFor(const EPinkCabCalibrationFixtureKind Kind)
{
    switch (Kind)
    {
    case EPinkCabCalibrationFixtureKind::SplitMu:
        return TEXT("PinkCab.Surface.SplitMu.DryWet");
    case EPinkCabCalibrationFixtureKind::RoughRoad:
        return TEXT("PinkCab.Surface.RoughRoad.Reference");
    default:
        return TEXT("PinkCab.Surface.DryAsphalt.Reference");
    }
}

FName EnvironmentFor(const EPinkCabCalibrationFixtureKind Kind)
{
    switch (Kind)
    {
    case EPinkCabCalibrationFixtureKind::GradeLaunch:
        return TEXT("PinkCab.FixtureEnvironment.Grade.Reference");
    case EPinkCabCalibrationFixtureKind::SplitMu:
        return TEXT("PinkCab.FixtureEnvironment.SplitMu.Reference");
    case EPinkCabCalibrationFixtureKind::RoughRoad:
        return TEXT("PinkCab.FixtureEnvironment.RoughRoad.Reference");
    default:
        return TEXT("PinkCab.FixtureEnvironment.ChaosWeave.FlatReference");
    }
}

FPinkCabCalibrationInputKeyframe Key(
    const double TimeSeconds,
    const float Throttle,
    const float Brake,
    const float Clutch,
    const float Handbrake,
    const float Steering,
    const int32 Gear)
{
    FPinkCabCalibrationInputKeyframe K;
    K.TimeSeconds = TimeSeconds;
    K.Throttle01 = Throttle;
    K.Brake01 = Brake;
    K.Clutch01 = Clutch;
    K.Handbrake01 = Handbrake;
    K.Steering = Steering;
    K.RequestedGear = Gear;
    return K;
}

TArray<FPinkCabCalibrationInputKeyframe> TraceFor(
    const EPinkCabCalibrationFixtureKind Kind)
{
    static const TArray<FPinkCabCalibrationInputKeyframe> Traces[] = {
        {Key(0.0,0,0,1,0,0,1), Key(0.5,0.25f,0,0.5f,0,0,1),
            Key(1.5,0.25f,0,0,0,0,1), Key(4.0,0,1,0,0,0,1)},
        {Key(0.0,0,0,1,0,0,1), Key(0.5,0.25f,0,0.5f,0,0,1),
            Key(1.5,0.25f,0,0,0,0,1), Key(4.0,0,1,0,0,0,1)},
        {Key(0.0,1,0,1,0,0,1), Key(1.0,1,0,0.75f,0,0,1),
            Key(2.0,1,0,0.5f,0,0,1), Key(3.0,1,0,0.25f,0,0,1),
            Key(4.0,1,0,0,0,0,1)},
        {Key(0.0,0.30f,0,0,0,0,2), Key(1.0,0.30f,0,0,0,0.30f,2),
            Key(5.0,0.30f,0,0,0,0.30f,2), Key(6.0,0,0,0,0,0,2)},
        {Key(0.0,0.30f,0,0,0,0.25f,2), Key(1.0,0,0.50f,0,0,0.25f,2),
            Key(3.0,0,0,0,0,0,2)},
        {Key(0.0,0.20f,0,0,0,0.25f,2), Key(1.0,0.60f,0,0,0,0.25f,2),
            Key(3.0,0,0,0,0,0,2)},
        {Key(0.0,0.50f,0,0,0,0.25f,3), Key(2.0,0,0,0,0,0.25f,3),
            Key(4.0,0,0,0,0,0,3)},
        {Key(0.0,0.35f,0,0,0,0,2), Key(2.0,0,0.75f,0,0,0,2),
            Key(4.0,0,0,0,0,0,2)},
        {Key(0.0,0.40f,0,0,0,0,3), Key(2.0,0,0.25f,0,0,0,3),
            Key(3.0,0,0.50f,0,0,0,3), Key(4.0,0,0.75f,0,0,0,3),
            Key(5.0,0,1,0,0,0,3)},
        {Key(0.0,0.30f,0,0,0,0,2), Key(0.75,0.30f,0,0,0,0.30f,2),
            Key(1.5,0.30f,0,0,0,-0.30f,2), Key(2.25,0.30f,0,0,0,0.30f,2),
            Key(3.0,0.30f,0,0,0,-0.30f,2), Key(3.75,0,0,0,0,0,2)},
        {Key(0.0,0.25f,0,0,0,0,2), Key(5.0,0.25f,0,0,0,0,2),
            Key(6.0,0,0,0,0,0,2)}
    };
    const int32 Index = static_cast<int32>(Kind);
    return Traces[FMath::Clamp(Index, 0, static_cast<int32>(UE_ARRAY_COUNT(Traces)) - 1)];
}

uint64 HashBytes(const FString& Text)
{
    FTCHARToUTF8 Utf8(*Text);
    uint64 Hash = 1469598103934665603ull;
    for (int32 Index = 0; Index < Utf8.Length(); ++Index)
    {
        Hash ^= static_cast<uint8>(Utf8.Get()[Index]);
        Hash *= 1099511628211ull;
    }
    return Hash;
}
}

bool FPinkCabCalibrationFixture::IsValid() const
{
    if (FixtureId.IsNone() || MapPath.IsEmpty() || EnvironmentId.IsNone()
        || SurfaceId.IsNone() || LoadFixtureId.IsNone() || ModelId.IsNone()
        || ProfileId.IsNone() || ProfileHash == 0 || LoadMassKg <= 0.0f
        || (FpsCap != 30 && FpsCap != 60 && FpsCap != 120)
        || InputTrace.Num() < 2 || InputTrace[0].TimeSeconds != 0.0)
    {
        return false;
    }
    double Previous = -1.0;
    for (const FPinkCabCalibrationInputKeyframe& K : InputTrace)
    {
        if (K.TimeSeconds <= Previous || K.Throttle01 < 0.0f || K.Throttle01 > 1.0f
            || K.Brake01 < 0.0f || K.Brake01 > 1.0f || K.Clutch01 < 0.0f
            || K.Clutch01 > 1.0f || K.Handbrake01 < 0.0f || K.Handbrake01 > 1.0f
            || K.Steering < -1.0f || K.Steering > 1.0f)
        {
            return false;
        }
        Previous = K.TimeSeconds;
    }
    return true;
}

FPinkCabCalibrationInputSample FPinkCabCalibrationFixture::SampleAtSeconds(
    const double TimeSeconds) const
{
    FPinkCabCalibrationInputSample Result;
    for (const FPinkCabCalibrationInputKeyframe& K : InputTrace)
    {
        if (K.TimeSeconds > TimeSeconds)
        {
            break;
        }
        Result.Throttle01 = K.Throttle01;
        Result.Brake01 = K.Brake01;
        Result.Clutch01 = K.Clutch01;
        Result.Handbrake01 = K.Handbrake01;
        Result.Steering = K.Steering;
        Result.RequestedGear = K.RequestedGear;
    }
    return Result;
}

uint64 FPinkCabCalibrationFixture::GetDeterministicHash() const
{
    FString Fingerprint = FString::Printf(
        TEXT("%s|%d|%s|%s|%s|%s|%s|%s|%d|%d|%016llX|%.3f|%d|%.9f"),
        *FixtureId.ToString(), static_cast<int32>(Kind), *MapPath,
        *EnvironmentId.ToString(), *SurfaceId.ToString(),
        *LoadFixtureId.ToString(), *ModelId.ToString(), *ProfileId.ToString(),
        ProfileSchemaVersion, CalibrationVersion,
        static_cast<unsigned long long>(ProfileHash), LoadMassKg, FpsCap,
        FixedDeltaSeconds);
    for (const FPinkCabCalibrationInputKeyframe& K : InputTrace)
    {
        Fingerprint += FString::Printf(
            TEXT("|%.9f:%.6f:%.6f:%.6f:%.6f:%.6f:%d"),
            K.TimeSeconds, K.Throttle01, K.Brake01, K.Clutch01,
            K.Handbrake01, K.Steering, K.RequestedGear);
    }
    return HashBytes(Fingerprint);
}

FString FPinkCabCalibrationFixture::ToCsv() const
{
    FString Csv = FString::Printf(
        TEXT("fixture_id,%s\nkind,%s\nmap,%s\nenvironment_id,%s\nsurface_id,%s\n")
        TEXT("load_fixture_id,%s\nload_mass_kg,%.3f\nmodel_id,%s\nprofile_id,%s\n")
        TEXT("profile_schema,%d\ncalibration_version,%d\nprofile_hash,%016llX\n")
        TEXT("fps_cap,%d\nfixed_dt_s,%.9f\ntrace_hash,%016llX\n"),
        *FixtureId.ToString(), KindToken(Kind), *MapPath, *EnvironmentId.ToString(),
        *SurfaceId.ToString(), *LoadFixtureId.ToString(), LoadMassKg, *ModelId.ToString(),
        *ProfileId.ToString(), ProfileSchemaVersion, CalibrationVersion,
        static_cast<unsigned long long>(ProfileHash), FpsCap, FixedDeltaSeconds,
        static_cast<unsigned long long>(GetDeterministicHash()));
    Csv += TEXT("time_s,throttle,brake,clutch,handbrake,steering,requested_gear\n");
    for (const FPinkCabCalibrationInputKeyframe& K : InputTrace)
    {
        Csv += FString::Printf(TEXT("%.9f,%.6f,%.6f,%.6f,%.6f,%.6f,%d\n"),
            K.TimeSeconds, K.Throttle01, K.Brake01, K.Clutch01,
            K.Handbrake01, K.Steering, K.RequestedGear);
    }
    return Csv;
}

FPinkCabCalibrationFixture FPinkCabCalibrationFixtureCatalog::Build(
    const EPinkCabCalibrationFixtureKind Kind,
    const int32 FpsCap)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    FPinkCabCalibrationFixture Fixture;
    Fixture.Kind = Kind;
    Fixture.FpsCap = FpsCap;
    Fixture.FixedDeltaSeconds = 1.0 / static_cast<double>(FMath::Max(FpsCap, 1));
    Fixture.FixtureId = FName(*FString::Printf(
        TEXT("PinkCab.PhysicsFixture.%s.%dFPS"), KindToken(Kind), FpsCap));
    Fixture.MapPath = CanonicalMap;
    Fixture.EnvironmentId = EnvironmentFor(Kind);
    Fixture.SurfaceId = SurfaceFor(Kind);
    Fixture.LoadFixtureId = TEXT("PinkCab.Load.ReferenceCrewFullFuel");
    Fixture.ModelId = Profile.ModelId;
    Fixture.ProfileId = Profile.ProfileId;
    Fixture.ProfileSchemaVersion = Profile.SchemaVersion;
    Fixture.CalibrationVersion = Profile.CalibrationVersion;
    Fixture.ProfileHash = Profile.GetDeterministicProfileHash();
    Fixture.LoadMassKg = Profile.ReferenceMassKg.Value;
    Fixture.InputTrace = TraceFor(Kind);
    return Fixture;
}

TArray<FPinkCabCalibrationFixture>
FPinkCabCalibrationFixtureCatalog::BuildCanonicalMatrix()
{
    const EPinkCabCalibrationFixtureKind Kinds[] = {
        EPinkCabCalibrationFixtureKind::FlatLaunch,
        EPinkCabCalibrationFixtureKind::GradeLaunch,
        EPinkCabCalibrationFixtureKind::LimiterCouplingSweep,
        EPinkCabCalibrationFixtureKind::ConstantRadius,
        EPinkCabCalibrationFixtureKind::CombinedBrakeTurn,
        EPinkCabCalibrationFixtureKind::CombinedThrottleTurn,
        EPinkCabCalibrationFixtureKind::LiftOff,
        EPinkCabCalibrationFixtureKind::SplitMu,
        EPinkCabCalibrationFixtureKind::Braking,
        EPinkCabCalibrationFixtureKind::Slalom,
        EPinkCabCalibrationFixtureKind::RoughRoad};
    const int32 FpsCaps[] = {30, 60, 120};
    TArray<FPinkCabCalibrationFixture> Result;
    Result.Reserve(UE_ARRAY_COUNT(Kinds) * UE_ARRAY_COUNT(FpsCaps));
    for (const auto Kind : Kinds)
    {
        for (const int32 Fps : FpsCaps)
        {
            Result.Add(Build(Kind, Fps));
        }
    }
    return Result;
}
