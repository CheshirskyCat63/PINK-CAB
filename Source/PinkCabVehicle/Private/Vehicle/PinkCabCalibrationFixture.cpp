#include "Vehicle/PinkCabCalibrationFixture.h"

#include "Vehicle/PinkCabChaosPhysicalProfile.h"

namespace
{
uint64 HashBytesFnv1a(const ANSICHAR* Data, const int32 Length)
{
    uint64 Hash = 1469598103934665603ull;
    for (int32 Index = 0; Index < Length; ++Index)
    {
        Hash ^= static_cast<uint8>(Data[Index]);
        Hash *= 1099511628211ull;
    }
    return Hash;
}

uint64 HashString(const FString& Value)
{
    const FTCHARToUTF8 Utf8(*Value);
    return HashBytesFnv1a(Utf8.Get(), Utf8.Length());
}

void AppendFloat(FString& Out, const float Value)
{
    Out += FString::Printf(TEXT("%.6f|"), Value);
}

FPinkCabCalibrationInputKeyframe Key(
    const float Time,
    const float Throttle,
    const float Brake,
    const float Clutch,
    const float Steering,
    const int32 Gear)
{
    FPinkCabCalibrationInputKeyframe K;
    K.TimeSeconds = Time;
    K.Throttle01 = Throttle;
    K.Brake01 = Brake;
    K.Clutch01 = Clutch;
    K.Steering = Steering;
    K.Gear = Gear;
    return K;
}

FPinkCabCalibrationFixtureSpec BaseFixture(
    const FPinkCabChaosPhysicalProfile& Profile,
    const TCHAR* Id,
    const int32 Seed)
{
    FPinkCabCalibrationFixtureSpec F;
    F.FixtureId = FName(Id);
    F.Seed = Seed;
    F.ModelId = Profile.ModelId; F.ProfileId = Profile.ProfileId;
    F.ProfileSchemaVersion = Profile.SchemaVersion; F.CalibrationVersion = Profile.CalibrationVersion;
    F.ProfileHash = Profile.GetDeterministicProfileHash();
    F.VehicleMassKg = Profile.ReferenceMassKg.Value;
    return F;
}

void AddFlatLaunch(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("FLAT_LAUNCH"), 41001);
    F.InputTrace = {
        Key(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0),
        Key(0.5f, 0.25f, 0.0f, 1.0f, 0.0f, 1),
        Key(1.0f, 0.50f, 0.0f, 0.50f, 0.0f, 1),
        Key(2.0f, 1.00f, 0.0f, 0.0f, 0.0f, 1),
        Key(5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1)};
    Out.Add(MoveTemp(F));
}

void AddGradeLaunch(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("GRADE_LAUNCH_8PCT"), 41002);
    F.SurfaceId = TEXT("DRY_ASPHALT_GRADE_8PCT");
    F.InputTrace = {
        Key(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0),
        Key(0.5f, 0.50f, 0.0f, 1.0f, 0.0f, 1),
        Key(1.0f, 0.65f, 0.0f, 0.55f, 0.0f, 1),
        Key(3.0f, 0.75f, 0.0f, 0.0f, 0.0f, 1),
        Key(6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1)};
    Out.Add(MoveTemp(F));
}

void AddCouplingSweep(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("LIMITER_COUPLING_SWEEP"), 41003);
    F.InputTrace = {
        Key(0.0f, 0.85f, 0.0f, 1.0f, 0.0f, 1),
        Key(1.0f, 0.85f, 0.0f, 0.50f, 0.0f, 1),
        Key(2.0f, 0.85f, 0.0f, 0.001f, 0.0f, 1),
        Key(3.0f, 0.85f, 0.0f, 0.0f, 0.0f, 1),
        Key(5.0f, 1.00f, 0.0f, 0.0f, 0.0f, 1)};
    Out.Add(MoveTemp(F));
}

void AddConstantRadius(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("CONSTANT_RADIUS"), 41004);
    F.InputTrace = {
        Key(0.0f, 0.35f, 0.0f, 0.0f, 0.0f, 2),
        Key(1.0f, 0.35f, 0.0f, 0.0f, 0.25f, 2),
        Key(8.0f, 0.35f, 0.0f, 0.0f, 0.25f, 2)};
    Out.Add(MoveTemp(F));
}

void AddCombinedBrakeTurn(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("COMBINED_BRAKE_TURN"), 41005);
    F.InputTrace = {
        Key(0.0f, 0.45f, 0.0f, 0.0f, 0.25f, 3),
        Key(2.0f, 0.0f, 0.50f, 0.0f, 0.25f, 3),
        Key(5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3)};
    Out.Add(MoveTemp(F));
}

void AddCombinedThrottleTurn(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("COMBINED_THROTTLE_TURN"), 41006);
    F.InputTrace = {
        Key(0.0f, 0.25f, 0.0f, 0.0f, 0.25f, 2),
        Key(2.0f, 0.65f, 0.0f, 0.0f, 0.25f, 2),
        Key(5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2)};
    Out.Add(MoveTemp(F));
}

void AddLiftOff(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("LIFT_OFF"), 41007);
    F.InputTrace = {
        Key(0.0f, 0.60f, 0.0f, 0.0f, 0.20f, 3),
        Key(3.0f, 0.0f, 0.0f, 0.0f, 0.20f, 3),
        Key(6.0f, 0.0f, 0.0f, 0.0f, -0.15f, 3)};
    Out.Add(MoveTemp(F));
}

void AddSplitMu(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("SPLIT_MU_BRAKE"), 41008);
    F.SurfaceId = TEXT("SPLIT_MU_DRY_WET");
    F.SurfaceFrictionScale = 0.65f;
    F.InputTrace = {
        Key(0.0f, 0.40f, 0.0f, 0.0f, 0.0f, 3),
        Key(2.0f, 0.0f, 1.0f, 0.0f, 0.0f, 3),
        Key(5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3)};
    Out.Add(MoveTemp(F));
}

void AddStraightBraking(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("STRAIGHT_BRAKING"), 41009);
    F.InputTrace = {
        Key(0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 3),
        Key(2.0f, 0.0f, 0.25f, 0.0f, 0.0f, 3),
        Key(3.0f, 0.0f, 0.50f, 0.0f, 0.0f, 3),
        Key(4.0f, 0.0f, 1.00f, 0.0f, 0.0f, 3)};
    Out.Add(MoveTemp(F));
}

void AddSlalom(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("SLALOM"), 41010);
    F.InputTrace = {
        Key(0.0f, 0.35f, 0.0f, 0.0f, 0.0f, 2),
        Key(1.0f, 0.35f, 0.0f, 0.0f, 0.30f, 2),
        Key(2.0f, 0.35f, 0.0f, 0.0f, -0.30f, 2),
        Key(3.0f, 0.35f, 0.0f, 0.0f, 0.30f, 2),
        Key(4.0f, 0.35f, 0.0f, 0.0f, -0.30f, 2)};
    Out.Add(MoveTemp(F));
}

void AddRoughRoad(
    TArray<FPinkCabCalibrationFixtureSpec>& Out,
    const FPinkCabChaosPhysicalProfile& Profile)
{
    auto F = BaseFixture(Profile, TEXT("ROUGH_ROAD"), 41011);
    F.SurfaceId = TEXT("ROUGH_ROAD_STANDARD");
    F.InputTrace = {
        Key(0.0f, 0.25f, 0.0f, 0.0f, 0.0f, 2),
        Key(2.0f, 0.35f, 0.0f, 0.0f, 0.10f, 2),
        Key(6.0f, 0.25f, 0.0f, 0.0f, -0.10f, 2)};
    Out.Add(MoveTemp(F));
}
}


uint64 FPinkCabCalibrationFixtureSpec::GetInputTraceHash() const
{
    FString Canonical;
    for (const FPinkCabCalibrationInputKeyframe& K : InputTrace)
    {
        AppendFloat(Canonical, K.TimeSeconds);
        AppendFloat(Canonical, K.Throttle01);
        AppendFloat(Canonical, K.Brake01);
        AppendFloat(Canonical, K.Clutch01);
        AppendFloat(Canonical, K.Steering);
        Canonical += FString::Printf(TEXT("%d|"), K.Gear);
    }
    return HashString(Canonical);
}

uint64 FPinkCabCalibrationFixtureSpec::GetDeterministicHash() const
{
    FString Canonical = FString::Printf(
        TEXT("%s|%d|%.9f|%d|%s|%s|%d|%d|%016llX|%.6f|%s|%s|%.6f|%s|%.6f|%.6f|%016llX|"),
        *FixtureId.ToString(),
        Seed,
        PhysicsDeltaSeconds,
        FpsCap,
        *ModelId.ToString(),
        *ProfileId.ToString(),
        ProfileSchemaVersion,
        CalibrationVersion,
        static_cast<unsigned long long>(ProfileHash),
        VehicleMassKg,
        *LoadStateId.ToString(),
        *SurfaceId.ToString(),
        SurfaceFrictionScale,
        *TireStateId.ToString(),
        TireTemperatureC,
        TireWear01,
        static_cast<unsigned long long>(GetInputTraceHash()));
    return HashString(Canonical);
}

FString FPinkCabCalibrationFixtureSpec::ToInputTraceCsv() const
{
    FString Csv = TEXT("time_s,throttle,brake,clutch,steering,gear\n");
    for (const FPinkCabCalibrationInputKeyframe& K : InputTrace)
    {
        Csv += FString::Printf(
            TEXT("%.6f,%.6f,%.6f,%.6f,%.6f,%d\n"),
            K.TimeSeconds,
            K.Throttle01,
            K.Brake01,
            K.Clutch01,
            K.Steering,
            K.Gear);
    }
    return Csv;
}

FPinkCabCalibrationRunDescriptor FPinkCabCalibrationRunDescriptor::FromFixture(
    const FPinkCabCalibrationFixtureSpec& Fixture,
    const int32 InRunOrdinal)
{
    FPinkCabCalibrationRunDescriptor R;
    R.FixtureId = Fixture.FixtureId;
    R.RunOrdinal = InRunOrdinal;
    R.Seed = Fixture.Seed;
    R.PhysicsDeltaSeconds = Fixture.PhysicsDeltaSeconds;
    R.FpsCap = Fixture.FpsCap;
    R.ModelId = Fixture.ModelId; R.ProfileId = Fixture.ProfileId;
    R.ProfileSchemaVersion = Fixture.ProfileSchemaVersion; R.CalibrationVersion = Fixture.CalibrationVersion;
    R.ProfileHash = Fixture.ProfileHash;
    R.VehicleMassKg = Fixture.VehicleMassKg;
    R.LoadStateId = Fixture.LoadStateId; R.SurfaceId = Fixture.SurfaceId;
    R.SurfaceFrictionScale = Fixture.SurfaceFrictionScale;
    R.TireStateId = Fixture.TireStateId;
    R.TireTemperatureC = Fixture.TireTemperatureC;
    R.TireWear01 = Fixture.TireWear01;
    R.InputTraceHash = Fixture.GetInputTraceHash();
    R.FixtureHash = Fixture.GetDeterministicHash();
    return R;
}

namespace
{
bool MatchesExecutionAndProfile(
    const FPinkCabCalibrationRunDescriptor& A,
    const FPinkCabCalibrationRunDescriptor& B)
{
    return A.FixtureId == B.FixtureId
        && A.Seed == B.Seed
        && FMath::IsNearlyEqual(A.PhysicsDeltaSeconds, B.PhysicsDeltaSeconds)
        && A.FpsCap == B.FpsCap
        && A.ModelId == B.ModelId
        && A.ProfileId == B.ProfileId
        && A.ProfileSchemaVersion == B.ProfileSchemaVersion
        && A.CalibrationVersion == B.CalibrationVersion
        && A.ProfileHash == B.ProfileHash;
}

bool MatchesLoadSurfaceTireAndTrace(
    const FPinkCabCalibrationRunDescriptor& A,
    const FPinkCabCalibrationRunDescriptor& B)
{
    return FMath::IsNearlyEqual(A.VehicleMassKg, B.VehicleMassKg)
        && A.LoadStateId == B.LoadStateId
        && A.SurfaceId == B.SurfaceId
        && FMath::IsNearlyEqual(A.SurfaceFrictionScale, B.SurfaceFrictionScale)
        && A.TireStateId == B.TireStateId
        && FMath::IsNearlyEqual(A.TireTemperatureC, B.TireTemperatureC)
        && FMath::IsNearlyEqual(A.TireWear01, B.TireWear01)
        && A.InputTraceHash == B.InputTraceHash
        && A.FixtureHash == B.FixtureHash;
}
}

bool FPinkCabCalibrationRunDescriptor::IsComparableTo(
    const FPinkCabCalibrationRunDescriptor& Other) const
{
    return MatchesExecutionAndProfile(*this, Other)
        && MatchesLoadSurfaceTireAndTrace(*this, Other);
}

FString FPinkCabCalibrationRunDescriptor::ToMetadataText() const
{
    return FString::Printf(
        TEXT("fixture_id=%s\nrun_ordinal=%d\nseed=%d\nphysics_dt=%.9f\nfps_cap=%d\n")
        TEXT("model_id=%s\nprofile_id=%s\nprofile_schema=%d\ncalibration=%d\nprofile_hash=%016llX\n")
        TEXT("vehicle_mass_kg=%.6f\nload_state=%s\nsurface_id=%s\nsurface_friction_scale=%.6f\n")
        TEXT("tire_state=%s\ntire_temperature_c=%.6f\ntire_wear=%.6f\ninput_trace_hash=%016llX\nfixture_hash=%016llX\n"),
        *FixtureId.ToString(),
        RunOrdinal,
        Seed,
        PhysicsDeltaSeconds,
        FpsCap,
        *ModelId.ToString(),
        *ProfileId.ToString(),
        ProfileSchemaVersion,
        CalibrationVersion,
        static_cast<unsigned long long>(ProfileHash),
        VehicleMassKg,
        *LoadStateId.ToString(),
        *SurfaceId.ToString(),
        SurfaceFrictionScale,
        *TireStateId.ToString(),
        TireTemperatureC,
        TireWear01,
        static_cast<unsigned long long>(InputTraceHash),
        static_cast<unsigned long long>(FixtureHash));
}

TArray<FPinkCabCalibrationFixtureSpec>
FPinkCabCalibrationFixtureLibrary::BuildCanonicalSet(
    const FPinkCabChaosPhysicalProfile& Profile)
{
    TArray<FPinkCabCalibrationFixtureSpec> Result;
    Result.Reserve(11);
    AddFlatLaunch(Result, Profile);
    AddGradeLaunch(Result, Profile);
    AddCouplingSweep(Result, Profile);
    AddConstantRadius(Result, Profile);
    AddCombinedBrakeTurn(Result, Profile);
    AddCombinedThrottleTurn(Result, Profile);
    AddLiftOff(Result, Profile);
    AddSplitMu(Result, Profile);
    AddStraightBraking(Result, Profile);
    AddSlalom(Result, Profile);
    AddRoughRoad(Result, Profile);
    return Result;
}
