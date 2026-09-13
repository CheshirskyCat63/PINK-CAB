#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabSuspendedBusRuntime.h"

namespace PinkCabSuspendedBusTests
{
FPinkCabSuspendedBusRuntime BuildRuntime()
{
    FPinkCabSuspendedBusRuntime Runtime(4, 4);
    Runtime.TryAddSegment(TEXT("bus:s0"), 100.0, false);
    Runtime.TryAddSegment(TEXT("bus:s1"), 200.0, true);
    Runtime.TryAddSegment(TEXT("bus:s2"), 100.0, false);
    Runtime.TryAddObstacle(TEXT("obs:a"), TEXT("bus:s1"), 25.0, 50.0);
    Runtime.TryAddObstacle(TEXT("obs:b"), TEXT("bus:s1"), 100.0, 125.0);
    return Runtime;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSuspendedBusProgressTest,
    "PinkCab.Vertical.L2.Bus.Progress",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSuspendedBusProgressTest::RunTest(const FString& Parameters)
{
    FPinkCabSuspendedBusRuntime Runtime = PinkCabSuspendedBusTests::BuildRuntime();
    TestTrue(TEXT("route starts"), Runtime.Start(TEXT("route:bus-main"), 50.0));    TestEqual(TEXT("starts first segment"), Runtime.GetCurrentSegmentId(), FString(TEXT("bus:s0")));
    TestEqual(TEXT("moving phase"), Runtime.GetPhase(), EPinkCabSuspendedBusPhase::Moving);
    TestFalse(TEXT("road contact not eligible"), Runtime.IsTatraContactEligible());

    Runtime.Advance(2.5);
    TestEqual(TEXT("crossed to second segment"), Runtime.GetCurrentSegmentId(), FString(TEXT("bus:s1")));
    TestTrue(TEXT("second segment contact eligible"), Runtime.IsTatraContactEligible());
    TestTrue(TEXT("distance carried"), FMath::IsNearlyEqual(Runtime.GetDistanceOnSegmentCm(), 25.0));
    TestEqual(TEXT("obstacle window phase"), Runtime.GetPhase(), EPinkCabSuspendedBusPhase::ObstacleWindow);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSuspendedBusObstacleOrderTest,
    "PinkCab.Vertical.L2.Bus.Obstacles",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSuspendedBusObstacleOrderTest::RunTest(const FString& Parameters)
{
    FPinkCabSuspendedBusRuntime Runtime = PinkCabSuspendedBusTests::BuildRuntime();
    TArray<FPinkCabSuspendedBusObstacle> Obstacles;
    TestTrue(TEXT("segment obstacles found"), Runtime.GetObstaclesForSegment(TEXT("bus:s1"), Obstacles));
    TestEqual(TEXT("two obstacles"), Obstacles.Num(), 2);
    TestEqual(TEXT("ordered first"), Obstacles[0].ObstacleId, FString(TEXT("obs:a")));
    TestEqual(TEXT("ordered second"), Obstacles[1].ObstacleId, FString(TEXT("obs:b")));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSuspendedBusValidationTest,
    "PinkCab.Vertical.L2.Bus.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSuspendedBusValidationTest::RunTest(const FString& Parameters)
{
    FPinkCabSuspendedBusRuntime Runtime(1, 1);
    TestFalse(TEXT("empty segment id rejected"), Runtime.TryAddSegment(TEXT(""), 100.0, false));
    TestFalse(TEXT("zero segment length rejected"), Runtime.TryAddSegment(TEXT("bus:bad"), 0.0, false));
    TestTrue(TEXT("valid segment"), Runtime.TryAddSegment(TEXT("bus:s0"), 100.0, true));
    TestFalse(TEXT("duplicate segment rejected"), Runtime.TryAddSegment(TEXT("bus:s0"), 100.0, true));
    TestFalse(TEXT("segment cap enforced"), Runtime.TryAddSegment(TEXT("bus:s1"), 100.0, false));
    TestFalse(TEXT("zero speed rejected"), Runtime.Start(TEXT("route:bus"), 0.0));
    TestFalse(TEXT("negative speed rejected"), Runtime.Start(TEXT("route:bus"), -1.0));
    TestTrue(TEXT("positive speed starts"), Runtime.Start(TEXT("route:bus"), 25.0));
    TestTrue(TEXT("one obstacle"), Runtime.TryAddObstacle(TEXT("obs:a"), TEXT("bus:s0"), 10.0, 20.0));
    TestFalse(TEXT("duplicate obstacle rejected"), Runtime.TryAddObstacle(TEXT("obs:a"), TEXT("bus:s0"), 10.0, 20.0));
    TestFalse(TEXT("obstacle cap enforced"), Runtime.TryAddObstacle(TEXT("obs:b"), TEXT("bus:s0"), 30.0, 40.0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSuspendedBusAbortTest,
    "PinkCab.Vertical.L2.Bus.AbortReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabSuspendedBusAbortTest::RunTest(const FString& Parameters)
{
    FPinkCabSuspendedBusRuntime Runtime = PinkCabSuspendedBusTests::BuildRuntime();
    TestTrue(TEXT("route starts"), Runtime.Start(TEXT("route:bus"), 50.0));
    Runtime.Advance(3.0);
    Runtime.Abort();
    TestEqual(TEXT("aborted phase"), Runtime.GetPhase(), EPinkCabSuspendedBusPhase::Aborted);
    TestTrue(TEXT("segment cleared"), Runtime.GetCurrentSegmentId().IsEmpty());
    TestEqual(TEXT("distance cleared"), Runtime.GetDistanceOnSegmentCm(), 0.0);
    TestFalse(TEXT("contact eligibility cleared"), Runtime.IsTatraContactEligible());
    Runtime.Reset();
    TestEqual(TEXT("reset idle"), Runtime.GetPhase(), EPinkCabSuspendedBusPhase::Idle);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSuspendedBusReconstructionTest,
    "PinkCab.Vertical.L2.Bus.Reconstruction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSuspendedBusReconstructionTest::RunTest(const FString& Parameters)
{
    FPinkCabSuspendedBusRuntime A = PinkCabSuspendedBusTests::BuildRuntime();
    FPinkCabSuspendedBusRuntime B = PinkCabSuspendedBusTests::BuildRuntime();
    TestTrue(TEXT("A starts"), A.Start(TEXT("route:bus"), 40.0));
    TestTrue(TEXT("B starts"), B.Start(TEXT("route:bus"), 40.0));
    A.Advance(4.25);
    B.Advance(4.25);
    TestEqual(TEXT("same segment after replay"),
        A.GetCurrentSegmentId(), B.GetCurrentSegmentId());
    TestTrue(TEXT("same distance after replay"),
        FMath::IsNearlyEqual(A.GetDistanceOnSegmentCm(), B.GetDistanceOnSegmentCm()));
    TestEqual(TEXT("same phase after replay"), A.GetPhase(), B.GetPhase());
    TestEqual(TEXT("reconstruction signature stable"),
        A.GetReconstructionSignature(), B.GetReconstructionSignature());
    TestEqual(TEXT("positive configured speed retained"), A.GetSpeedCmPerSec(), 40.0);
    TestFalse(TEXT("runtime never owns steering"), A.HasSteeringAuthority());
    TestFalse(TEXT("runtime never owns braking"), A.HasBrakingAuthority());
    return true;
}

#endif
