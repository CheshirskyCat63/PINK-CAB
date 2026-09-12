#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Traffic/PinkCabTrafficEntity.h"
#include "Traffic/PinkCabTrafficFlow.h"
#include "Traffic/PinkCabTrafficMaterializationPolicy.h"
#include "World/PinkCabCityIdentity.h"
#include "World/PinkCabChunkId.h"
#include "World/PinkCabRoadGraph.h"

static FPinkCabLaneId MakeTestLane()
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("TRAFFIC-TEST"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabChunkId Chunk = FPinkCabChunkId::From(City, {0, 0, 0});
    return FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, 0);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficLogicalProgressTest,
    "PinkCab.Traffic.LogicalProgress",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficLogicalProgressTest::RunTest(const FString& Parameters)
{
    FPinkCabTrafficEntity Entity(TEXT("traffic-1"), MakeTestLane(), 1000.0, 1500.0);
    Entity.Advance(2.0);
    TestEqual(TEXT("logical position advances without Actor"), Entity.GetLongitudinalCm(), 4000.0);
    TestEqual(TEXT("speed remains logical data"), Entity.GetSpeedCmPerSec(), 1500.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficBoundedPopulationTest,
    "PinkCab.Traffic.BoundedPopulation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficBoundedPopulationTest::RunTest(const FString& Parameters)
{
    FPinkCabTrafficFlowConstraints Constraints;
    Constraints.MaxLogicalEntities = 2;
    Constraints.MinPlayableGapCm = 500.0;
    FPinkCabTrafficFlow Flow(Constraints);
    const FPinkCabLaneId Lane = MakeTestLane();
    TestTrue(TEXT("first entity accepted"), Flow.TryAddOrdinary({TEXT("a"), Lane, 0.0, 1000.0}));
    TestTrue(TEXT("second entity accepted"), Flow.TryAddOrdinary({TEXT("b"), Lane, 1000.0, 1000.0}));
    TestFalse(TEXT("population ceiling enforced"), Flow.TryAddOrdinary({TEXT("c"), Lane, 2000.0, 1000.0}));
    TestEqual(TEXT("logical population remains bounded"), Flow.Num(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficPlayableGapTest,
    "PinkCab.Traffic.PlayableGap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficPlayableGapTest::RunTest(const FString& Parameters)
{
    FPinkCabTrafficFlowConstraints Constraints;
    Constraints.MaxLogicalEntities = 8;
    Constraints.MinPlayableGapCm = 800.0;
    FPinkCabTrafficFlow Flow(Constraints);
    const FPinkCabLaneId Lane = MakeTestLane();
    TestTrue(TEXT("seed entity accepted"), Flow.TryAddOrdinary({TEXT("seed"), Lane, 1000.0, 1200.0}));
    TestFalse(TEXT("too-close entity rejected"), Flow.TryAddOrdinary({TEXT("tight"), Lane, 1500.0, 1200.0}));
    TestTrue(TEXT("playable gap entity accepted"), Flow.TryAddOrdinary({TEXT("gap"), Lane, 2000.0, 1200.0}));
    TestEqual(TEXT("only valid spacing remains"), Flow.Num(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficNoStandingJamGenerationTest,
    "PinkCab.Traffic.NoDesignedStandingJam",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficNoStandingJamGenerationTest::RunTest(const FString& Parameters)
{
    FPinkCabTrafficFlowConstraints Constraints;
    Constraints.MaxLogicalEntities = 4;
    Constraints.MinPlayableGapCm = 500.0;
    FPinkCabTrafficFlow Flow(Constraints);
    const FPinkCabLaneId Lane = MakeTestLane();
    TestFalse(TEXT("ordinary generated flow rejects zero speed"), Flow.TryAddOrdinary({TEXT("stopped"), Lane, 0.0, 0.0}));
    TestFalse(TEXT("ordinary generated flow rejects reverse speed"), Flow.TryAddOrdinary({TEXT("reverse"), Lane, 1000.0, -1.0}));
    TestTrue(TEXT("moving ordinary traffic accepted"), Flow.TryAddOrdinary({TEXT("moving"), Lane, 0.0, 1.0}));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficMaterializationBoundaryTest,
    "PinkCab.Traffic.MaterializationBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficMaterializationBoundaryTest::RunTest(const FString& Parameters)
{
    const FPinkCabLaneId Lane = MakeTestLane();
    const FPinkCabTrafficEntity Entity(TEXT("traffic-materialize"), Lane, 2500.0, 900.0);
    FPinkCabTrafficMaterializationRequest Request;
    TestFalse(TEXT("external policy may keep entity logical"),
        FPinkCabTrafficMaterializationPolicy::TryBuildRequest(Entity, false, Request));
    TestTrue(TEXT("external policy may request materialization"),
        FPinkCabTrafficMaterializationPolicy::TryBuildRequest(Entity, true, Request));
    TestEqual(TEXT("request keeps stable traffic id"), Request.TrafficId, FString(TEXT("traffic-materialize")));
    TestTrue(TEXT("request keeps logical lane id"), Request.LaneId == Lane);
    TestEqual(TEXT("request keeps logical position"), Request.LongitudinalCm, 2500.0);
    return true;
}

#endif
