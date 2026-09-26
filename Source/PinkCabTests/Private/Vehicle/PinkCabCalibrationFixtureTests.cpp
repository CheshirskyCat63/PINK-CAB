#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabCalibrationFixture.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCalibrationFixtureCanonicalSetTest,
    "PinkCab.Vehicle.Physics.Fixtures.CanonicalSet",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCalibrationFixtureCanonicalSetTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const TArray<FPinkCabCalibrationFixtureSpec> Fixtures =
        FPinkCabCalibrationFixtureLibrary::BuildCanonicalSet(Profile);

    TestTrue(TEXT("canonical fixture set covers required calibration families"),
        Fixtures.Num() >= 11);

    TSet<FName> Ids;
    for (const FPinkCabCalibrationFixtureSpec& Fixture : Fixtures)
    {
        TestTrue(TEXT("fixture has stable id"), !Fixture.FixtureId.IsNone());
        TestTrue(TEXT("fixture has deterministic seed"), Fixture.Seed != 0);
        TestTrue(TEXT("fixture records positive physics dt"), Fixture.PhysicsDeltaSeconds > 0.0f);
        TestTrue(TEXT("fixture records positive fps cap"), Fixture.FpsCap > 0);
        TestEqual(TEXT("fixture profile id matches active profile"), Fixture.ProfileId, Profile.ProfileId);
        TestEqual(TEXT("fixture profile hash matches active profile"),
            Fixture.ProfileHash, Profile.GetDeterministicProfileHash());
        TestTrue(TEXT("fixture records surface"), !Fixture.SurfaceId.IsNone());
        TestTrue(TEXT("fixture records load"), Fixture.VehicleMassKg > 0.0f);
        TestTrue(TEXT("fixture records exact input trace"), Fixture.InputTrace.Num() >= 2);
        TestTrue(TEXT("fixture hash is non-zero"), Fixture.GetDeterministicHash() != 0ull);
        Ids.Add(Fixture.FixtureId);
    }
    TestEqual(TEXT("fixture ids are unique"), Ids.Num(), Fixtures.Num());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCalibrationFixtureFiveRunComparabilityTest,
    "PinkCab.Vehicle.Physics.Fixtures.FiveRunComparability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCalibrationFixtureFiveRunComparabilityTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const FPinkCabCalibrationFixtureSpec Fixture =
        FPinkCabCalibrationFixtureLibrary::BuildCanonicalSet(Profile)[0];

    TArray<FPinkCabCalibrationRunDescriptor> Runs;
    for (int32 Index = 0; Index < 5; ++Index)
    {
        Runs.Add(FPinkCabCalibrationRunDescriptor::FromFixture(Fixture, Index + 1));
    }

    for (int32 Index = 1; Index < Runs.Num(); ++Index)
    {
        TestTrue(TEXT("repeated fixture runs remain comparable"),
            Runs[0].IsComparableTo(Runs[Index]));
        TestEqual(TEXT("fixture hash is stable across repeats"),
            Runs[0].FixtureHash, Runs[Index].FixtureHash);
    }

    FPinkCabCalibrationRunDescriptor Changed = Runs[4];
    Changed.FpsCap = 30;
    TestFalse(TEXT("fps change invalidates comparability"), Runs[0].IsComparableTo(Changed));
    Changed = Runs[4];
    Changed.Seed += 1;
    TestFalse(TEXT("seed change invalidates comparability"), Runs[0].IsComparableTo(Changed));
    Changed = Runs[4];
    Changed.VehicleMassKg += 1.0f;
    TestFalse(TEXT("load change invalidates comparability"), Runs[0].IsComparableTo(Changed));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCalibrationFixtureEvidenceContractTest,
    "PinkCab.Vehicle.Physics.Fixtures.EvidenceContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCalibrationFixtureEvidenceContractTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const FPinkCabCalibrationFixtureSpec Fixture =
        FPinkCabCalibrationFixtureLibrary::BuildCanonicalSet(Profile)[0];
    const FPinkCabCalibrationRunDescriptor Run =
        FPinkCabCalibrationRunDescriptor::FromFixture(Fixture, 1);

    const FString Metadata = Run.ToMetadataText();
    TestTrue(TEXT("metadata records fixture id"), Metadata.Contains(TEXT("fixture_id=")));
    TestTrue(TEXT("metadata records physics dt"), Metadata.Contains(TEXT("physics_dt=")));
    TestTrue(TEXT("metadata records fps"), Metadata.Contains(TEXT("fps_cap=")));
    TestTrue(TEXT("metadata records profile hash"), Metadata.Contains(TEXT("profile_hash=")));
    TestTrue(TEXT("metadata records load"), Metadata.Contains(TEXT("vehicle_mass_kg=")));
    TestTrue(TEXT("metadata records surface"), Metadata.Contains(TEXT("surface_id=")));
    TestTrue(TEXT("metadata records tire state"), Metadata.Contains(TEXT("tire_state=")));
    TestTrue(TEXT("metadata records input trace hash"), Metadata.Contains(TEXT("input_trace_hash=")));

    const FString TraceCsv = Fixture.ToInputTraceCsv();
    TestTrue(TEXT("trace csv has timestamp"), TraceCsv.Contains(TEXT("time_s")));
    TestTrue(TEXT("trace csv has throttle"), TraceCsv.Contains(TEXT("throttle")));
    TestTrue(TEXT("trace csv has brake"), TraceCsv.Contains(TEXT("brake")));
    TestTrue(TEXT("trace csv has clutch"), TraceCsv.Contains(TEXT("clutch")));
    TestTrue(TEXT("trace csv has steering"), TraceCsv.Contains(TEXT("steering")));
    TestTrue(TEXT("trace csv has gear"), TraceCsv.Contains(TEXT("gear")));
    return true;
}

#endif
