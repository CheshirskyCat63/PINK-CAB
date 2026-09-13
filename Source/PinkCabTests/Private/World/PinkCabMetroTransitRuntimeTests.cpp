#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabMetroTransitRuntime.h"

namespace PinkCabMetroTransitTests
{
struct FFixture
{
    FPinkCabCityIdentity City;
    FPinkCabMetroTransitRuntime Runtime;
    FString A;
    FString B;
    FString C;
    FString AB;
    FString BC;

    FFixture()
        : City(FPinkCabCityIdentity::Create(TEXT("VERT-METRO"), TEXT("gen-1"), TEXT("content-1")))
        , Runtime(4, 4)
    {
        Runtime.TryAddStation(City, TEXT("station-a"), A);
        Runtime.TryAddStation(City, TEXT("station-b"), B);
        Runtime.TryAddStation(City, TEXT("station-c"), C);
        Runtime.TryAddSegment(City, A, B, TEXT("ab"), 10.0, 2.0, 4.0, AB);
        Runtime.TryAddSegment(City, B, C, TEXT("bc"), 8.0, 1.0, 2.0, BC);
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMetroGraphTest,
    "PinkCab.Vertical.L2.Metro.Graph",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMetroGraphTest::RunTest(const FString& Parameters)
{
    PinkCabMetroTransitTests::FFixture F;
    TArray<FString> Outgoing;
    TestTrue(TEXT("station B outgoing resolves"), F.Runtime.GetOutgoingSegments(F.B, Outgoing));
    TestEqual(TEXT("one outgoing from B"), Outgoing.Num(), 1);
    TestEqual(TEXT("B connects to BC"), Outgoing[0], F.BC);

    PinkCabMetroTransitTests::FFixture G;
    TestEqual(TEXT("station identity reconstructs"), F.A, G.A);
    TestEqual(TEXT("segment identity reconstructs"), F.AB, G.AB);
    TestEqual(TEXT("definition signatures reconstruct"),
        F.Runtime.GetDefinitionSignature(), G.Runtime.GetDefinitionSignature());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMetroTimingTest,
    "PinkCab.Vertical.L2.Metro.Timing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMetroTimingTest::RunTest(const FString& Parameters)
{
    PinkCabMetroTransitTests::FFixture F;
    TestTrue(TEXT("segment starts"), F.Runtime.Start(TEXT("metro:main"), F.AB));
    TestEqual(TEXT("starts at A"), F.Runtime.GetCurrentStationId(), F.A);
    TestEqual(TEXT("AB active"), F.Runtime.GetCurrentSegmentId(), F.AB);
    TestTrue(TEXT("metro default non-driveable"), !F.Runtime.IsMetroDriveableForTatra());

    TestTrue(TEXT("advance accepted"), F.Runtime.Advance(4.5));
    TestTrue(TEXT("segment elapsed deterministic"),
        FMath::IsNearlyEqual(F.Runtime.GetSegmentElapsedSeconds(), 4.5));
    TestTrue(TEXT("schedule time deterministic"),
        FMath::IsNearlyEqual(F.Runtime.GetScheduleTimeSeconds(), 4.5));

    TestTrue(TEXT("finish segment"), F.Runtime.Advance(5.5));
    TestEqual(TEXT("arrives at B"), F.Runtime.GetCurrentStationId(), F.B);
    TestTrue(TEXT("segment cleared at station"), F.Runtime.GetCurrentSegmentId().IsEmpty());
    TestTrue(TEXT("ten second schedule"),
        FMath::IsNearlyEqual(F.Runtime.GetScheduleTimeSeconds(), 10.0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMetroTransitionWindowTest,
    "PinkCab.Vertical.L2.Metro.TransitionWindow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMetroTransitionWindowTest::RunTest(const FString& Parameters)
{
    PinkCabMetroTransitTests::FFixture F;
    TestTrue(TEXT("segment starts"), F.Runtime.Start(TEXT("metro:main"), F.AB));
    TestEqual(TEXT("initial tatra state"), F.Runtime.GetTatraState(), EPinkCabMetroTatraState::Road);
    TestFalse(TEXT("cannot enter contact before window"), F.Runtime.EnterTransitContact());

    F.Runtime.Advance(2.5);
    TestTrue(TEXT("window refresh"), F.Runtime.RefreshTatraTransitionState());
    TestEqual(TEXT("window becomes eligible"), F.Runtime.GetTatraState(), EPinkCabMetroTatraState::TransitionEligible);
    TestFalse(TEXT("eligible is still non-driveable"), F.Runtime.IsMetroDriveableForTatra());
    TestTrue(TEXT("approved contact entry"), F.Runtime.EnterTransitContact());
    TestEqual(TEXT("contact state"), F.Runtime.GetTatraState(), EPinkCabMetroTatraState::TransitContact);
    TestTrue(TEXT("contact explicitly driveable"), F.Runtime.IsMetroDriveableForTatra());

    TestTrue(TEXT("exit request"), F.Runtime.RequestExit());
    TestEqual(TEXT("exit pending"), F.Runtime.GetTatraState(), EPinkCabMetroTatraState::ExitPending);
    TestTrue(TEXT("complete exit"), F.Runtime.CompleteExit());
    TestEqual(TEXT("returns road"), F.Runtime.GetTatraState(), EPinkCabMetroTatraState::Road);
    TestFalse(TEXT("road metro non-driveable"), F.Runtime.IsMetroDriveableForTatra());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMetroMissedWindowTest,
    "PinkCab.Vertical.L2.Metro.MissedWindow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMetroMissedWindowTest::RunTest(const FString& Parameters)
{
    PinkCabMetroTransitTests::FFixture F;
    TestTrue(TEXT("segment starts"), F.Runtime.Start(TEXT("metro:main"), F.AB));
    F.Runtime.Advance(5.0);
    TestFalse(TEXT("missed window not eligible"), F.Runtime.RefreshTatraTransitionState());
    TestEqual(TEXT("missed window stays road"), F.Runtime.GetTatraState(), EPinkCabMetroTatraState::Road);
    TestFalse(TEXT("missed window contact rejected"), F.Runtime.EnterTransitContact());
    TestFalse(TEXT("missed window non-driveable"), F.Runtime.IsMetroDriveableForTatra());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMetroAbortReconstructionTest,
    "PinkCab.Vertical.L2.Metro.AbortReconstruction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMetroAbortReconstructionTest::RunTest(const FString& Parameters)
{
    PinkCabMetroTransitTests::FFixture A;
    PinkCabMetroTransitTests::FFixture B;
    TestTrue(TEXT("A starts"), A.Runtime.Start(TEXT("metro:main"), A.AB));
    TestTrue(TEXT("B starts"), B.Runtime.Start(TEXT("metro:main"), B.AB));
    A.Runtime.Advance(2.5);
    B.Runtime.Advance(2.5);
    A.Runtime.RefreshTatraTransitionState();
    B.Runtime.RefreshTatraTransitionState();
    TestEqual(TEXT("state signature reconstructs"),
        A.Runtime.GetReconstructionSignature(), B.Runtime.GetReconstructionSignature());

    A.Runtime.Abort();
    TestEqual(TEXT("abort state"), A.Runtime.GetTatraState(), EPinkCabMetroTatraState::Aborted);
    TestTrue(TEXT("abort clears segment"), A.Runtime.GetCurrentSegmentId().IsEmpty());
    TestFalse(TEXT("abort non-driveable"), A.Runtime.IsMetroDriveableForTatra());
    A.Runtime.Reset();
    TestEqual(TEXT("reset road"), A.Runtime.GetTatraState(), EPinkCabMetroTatraState::Road);
    TestTrue(TEXT("reset clears route"), A.Runtime.GetActiveRouteId().IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMetroValidationTest,
    "PinkCab.Vertical.L2.Metro.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMetroValidationTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("VERT-METRO"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabMetroTransitRuntime Runtime(2, 1);
    FString A, B, C, AB;
    TestTrue(TEXT("A added"), Runtime.TryAddStation(City, TEXT("a"), A));
    TestTrue(TEXT("B added"), Runtime.TryAddStation(City, TEXT("b"), B));
    TestFalse(TEXT("station cap enforced"), Runtime.TryAddStation(City, TEXT("c"), C));
    FString DuplicateOut = TEXT("preserve");
    TestFalse(TEXT("duplicate station rejected"), Runtime.TryAddStation(City, TEXT("a"), DuplicateOut));
    TestEqual(TEXT("failed station add preserves output"), DuplicateOut, FString(TEXT("preserve")));

    TestFalse(TEXT("zero timing rejected"), Runtime.TryAddSegment(
        City, A, B, TEXT("bad"), 0.0, 1.0, 2.0, AB));
    TestFalse(TEXT("reversed window rejected"), Runtime.TryAddSegment(
        City, A, B, TEXT("bad-window"), 10.0, 4.0, 2.0, AB));
    TestTrue(TEXT("valid segment added"), Runtime.TryAddSegment(
        City, A, B, TEXT("ab"), 10.0, 2.0, 4.0, AB));
    TestFalse(TEXT("segment cap enforced"), Runtime.TryAddSegment(
        City, B, A, TEXT("ba"), 10.0, 2.0, 4.0, C));
    TestFalse(TEXT("unknown segment start rejected"), Runtime.Start(
        TEXT("metro:main"), TEXT("segment:missing")));
    return true;
}

#endif
