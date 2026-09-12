#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabChaosWeaveCourse.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosWeaveLayoutTest,
    "PinkCab.World.ChaosWeaveCourse.Layout",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosWeaveLayoutTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("course has twelve vehicle blocks"),
        APinkCabChaosWeaveCourse::GetObstacleCount(), 12);

    const float Gap = APinkCabChaosWeaveCourse::GetLongitudinalGapCm();
    TestTrue(TEXT("blocks leave useful braking/weave distance"), Gap >= 1600.0f);

    int32 PreviousSide = 0;
    for (int32 Index = 0; Index < APinkCabChaosWeaveCourse::GetObstacleCount(); ++Index)
    {
        const FVector Location = APinkCabChaosWeaveCourse::GetObstacleLocation(Index);
        const int32 Side = Location.Y < 0.0f ? -1 : 1;
        if (Index > 0)
        {
            TestTrue(TEXT("obstacles alternate road sides"), Side != PreviousSide);
        }
        PreviousSide = Side;
    }

    const FTransform Spawn = APinkCabChaosWeaveCourse::GetPawnSpawnTransform();
    TestTrue(TEXT("spawn starts before first obstacle"),
        Spawn.GetLocation().X < APinkCabChaosWeaveCourse::GetObstacleLocation(0).X);
    TestTrue(TEXT("spawn is above road surface"), Spawn.GetLocation().Z > 50.0f);

    const FVector Bounds = APinkCabChaosWeaveCourse::GetCourseSizeCm();
    TestTrue(TEXT("course is long enough for high-speed weave"), Bounds.X >= 24000.0f);
    TestTrue(TEXT("course is wide enough for two-sided passing"), Bounds.Y >= 1000.0f);
    return true;
}

#endif
