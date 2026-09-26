#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabCalibrationFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicsFixtureCatalogTest,
    "PinkCab.Vehicle.Physics.Fixtures.CanonicalCatalog",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPhysicsFixtureCatalogTest::RunTest(const FString& Parameters)
{
    const TArray<FPinkCabCalibrationFixture> Fixtures =
        FPinkCabCalibrationFixtureCatalog::BuildCanonicalMatrix();

    TestEqual(TEXT("11 scenarios x 3 FPS caps"), Fixtures.Num(), 33);

    TSet<EPinkCabCalibrationFixtureKind> Kinds;
    TSet<int32> FpsCaps;
    for (const FPinkCabCalibrationFixture& Fixture : Fixtures)
    {
        Kinds.Add(Fixture.Kind);
        FpsCaps.Add(Fixture.FpsCap);
        TestTrue(TEXT("fixture validates"), Fixture.IsValid());
        TestFalse(TEXT("fixture id declared"), Fixture.FixtureId.IsNone());
        TestFalse(TEXT("profile id declared"), Fixture.ProfileId.IsNone());
        TestTrue(TEXT("profile hash declared"), Fixture.ProfileHash != 0);
        TestFalse(TEXT("load id declared"), Fixture.LoadFixtureId.IsNone());
        TestFalse(TEXT("surface id declared"), Fixture.SurfaceId.IsNone());
        TestFalse(TEXT("environment id declared"), Fixture.EnvironmentId.IsNone());
        TestTrue(TEXT("absolute input trace has at least two keyframes"),
            Fixture.InputTrace.Num() >= 2);
    }

    TestEqual(TEXT("all required scenario kinds exist"), Kinds.Num(), 11);
    TestTrue(TEXT("30 FPS fixture set exists"), FpsCaps.Contains(30));
    TestTrue(TEXT("60 FPS fixture set exists"), FpsCaps.Contains(60));
    TestTrue(TEXT("120 FPS fixture set exists"), FpsCaps.Contains(120));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicsFixtureRepeatabilityTest,
    "PinkCab.Vehicle.Physics.Fixtures.FiveRepeatDeterminism",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPhysicsFixtureRepeatabilityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCalibrationFixture Fixture =
        FPinkCabCalibrationFixtureCatalog::Build(
            EPinkCabCalibrationFixtureKind::FlatLaunch, 60);

    const uint64 ExpectedHash = Fixture.GetDeterministicHash();
    TestTrue(TEXT("fixture hash is nonzero"), ExpectedHash != 0);

    for (int32 Repeat = 0; Repeat < 5; ++Repeat)
    {
        const FPinkCabCalibrationFixture Again =
            FPinkCabCalibrationFixtureCatalog::Build(
                EPinkCabCalibrationFixtureKind::FlatLaunch, 60);
        TestEqual(TEXT("same fixture reconstructs exact hash"),
            Again.GetDeterministicHash(), ExpectedHash);

        const FPinkCabCalibrationInputSample A = Again.SampleAtSeconds(1.25);
        const FPinkCabCalibrationInputSample B = Fixture.SampleAtSeconds(1.25);
        TestTrue(TEXT("same absolute timestamp reproduces throttle"),
            FMath::IsNearlyEqual(A.Throttle01, B.Throttle01, 1.0e-6f));
        TestTrue(TEXT("same absolute timestamp reproduces clutch"),
            FMath::IsNearlyEqual(A.Clutch01, B.Clutch01, 1.0e-6f));
        TestTrue(TEXT("same absolute timestamp reproduces steering"),
            FMath::IsNearlyEqual(A.Steering, B.Steering, 1.0e-6f));
        TestEqual(TEXT("same absolute timestamp reproduces gear"),
            A.RequestedGear, B.RequestedGear);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicsFixtureFpsIndependenceTest,
    "PinkCab.Vehicle.Physics.Fixtures.AbsoluteTraceAcrossFps",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPhysicsFixtureFpsIndependenceTest::RunTest(const FString& Parameters)
{
    const FPinkCabCalibrationFixture At30 =
        FPinkCabCalibrationFixtureCatalog::Build(
            EPinkCabCalibrationFixtureKind::Slalom, 30);
    const FPinkCabCalibrationFixture At60 =
        FPinkCabCalibrationFixtureCatalog::Build(
            EPinkCabCalibrationFixtureKind::Slalom, 60);
    const FPinkCabCalibrationFixture At120 =
        FPinkCabCalibrationFixtureCatalog::Build(
            EPinkCabCalibrationFixtureKind::Slalom, 120);

    for (const double Time : {0.0, 0.75, 1.5, 2.25, 3.0})
    {
        const auto A = At30.SampleAtSeconds(Time);
        const auto B = At60.SampleAtSeconds(Time);
        const auto C = At120.SampleAtSeconds(Time);
        TestEqual(TEXT("30/60 throttle same at absolute time"), A.Throttle01, B.Throttle01);
        TestEqual(TEXT("60/120 throttle same at absolute time"), B.Throttle01, C.Throttle01);
        TestEqual(TEXT("30/60 steering same at absolute time"), A.Steering, B.Steering);
        TestEqual(TEXT("60/120 steering same at absolute time"), B.Steering, C.Steering);
        TestEqual(TEXT("30/60 gear same at absolute time"), A.RequestedGear, B.RequestedGear);
        TestEqual(TEXT("60/120 gear same at absolute time"), B.RequestedGear, C.RequestedGear);
    }

    TestTrue(TEXT("CSV records FPS metadata"), At60.ToCsv().Contains(TEXT("fps_cap,60")));
    TestTrue(TEXT("CSV records profile identity"), At60.ToCsv().Contains(TEXT("profile_id")));
    TestTrue(TEXT("CSV records surface identity"), At60.ToCsv().Contains(TEXT("surface_id")));
    TestTrue(TEXT("CSV records load identity"), At60.ToCsv().Contains(TEXT("load_fixture_id")));
    return true;
}

#endif
