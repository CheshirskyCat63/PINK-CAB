#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabRouteService.h"
#include "World/PinkCabCityDeltaState.h"
#include "World/PinkCabRoadGraph.h"

namespace
{
FPinkCabLogicalLane Lane(
    const TCHAR* Id,
    const TCHAR* From,
    const TCHAR* To,
    double LengthCm,
    int32 Layer = 0)
{
    FPinkCabLogicalLane Result;
    Result.LaneId = FPinkCabLaneId(Id);
    Result.FromNode = FPinkCabRoadNodeId(From);
    Result.ToNode = FPinkCabRoadNodeId(To);
    Result.LengthCm = LengthCm;
    Result.Layer = Layer;
    return Result;
}

FPinkCabRoadGraph BuildTieGraph()
{
    FPinkCabRoadGraph Graph;
    Graph.AddLane(Lane(TEXT("lane:start"), TEXT("n0"), TEXT("n1"), 10.0));
    Graph.AddLane(Lane(TEXT("lane:b"), TEXT("n1"), TEXT("n3"), 100.0));
    Graph.AddLane(Lane(TEXT("lane:b-join"), TEXT("n3"), TEXT("n4"), 100.0));
    Graph.AddLane(Lane(TEXT("lane:a"), TEXT("n1"), TEXT("n2"), 100.0));
    Graph.AddLane(Lane(TEXT("lane:a-join"), TEXT("n2"), TEXT("n4"), 100.0));
    Graph.AddLane(Lane(TEXT("lane:goal"), TEXT("n4"), TEXT("n5"), 10.0));
    return Graph;
}

FPinkCabRouteRequest Request(const TCHAR* Start, const TCHAR* Goal, int32 Budget = 64)
{
    FPinkCabRouteRequest R;
    R.StartLaneId = FPinkCabLaneId(Start);
    R.GoalLaneId = FPinkCabLaneId(Goal);
    R.MaxVisitedNodes = Budget;
    return R;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRouteShortestTieBreakTest,
    "PinkCab.World.Routing.ShortestStableTieBreak",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRouteShortestTieBreakTest::RunTest(const FString& Parameters)
{
    const FPinkCabRoadGraph Graph = BuildTieGraph();
    const FPinkCabCityDeltaState Deltas;
    FPinkCabRoute Route;
    TestTrue(TEXT("route found"), FPinkCabRouteService::FindRoute(Graph, Request(TEXT("lane:start"), TEXT("lane:goal")), Deltas, Route));
    TestEqual(TEXT("route contains four lanes"), Route.LaneIds.Num(), 4);
    TestEqual(TEXT("lexicographically stable equal-cost branch wins"), Route.LaneIds[1].Serialize(), FString(TEXT("lane:a")));
    TestEqual(TEXT("matching join lane retained"), Route.LaneIds[2].Serialize(), FString(TEXT("lane:a-join")));
    TestEqual(TEXT("total route length sums logical lanes"), Route.TotalLengthCm, 220.0);
    TestFalse(TEXT("route signature populated"), Route.RouteSignature.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRouteClosureExclusionTest,
    "PinkCab.World.Routing.ClosedLaneExclusion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRouteClosureExclusionTest::RunTest(const FString& Parameters)
{
    const FPinkCabRoadGraph Graph = BuildTieGraph();
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("ROUTE"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabCityDeltaState Deltas;
    FString DeltaId;
    TestTrue(TEXT("closure accepted"),
        Deltas.TryAddLaneClosure(City, FPinkCabLaneId(TEXT("lane:a")), TEXT("incident-a"), DeltaId));
    FPinkCabRoute Route;
    TestTrue(TEXT("alternate route remains reachable"),
        FPinkCabRouteService::FindRoute(Graph, Request(TEXT("lane:start"), TEXT("lane:goal")), Deltas, Route));
    TestEqual(TEXT("closed branch excluded"), Route.LaneIds[1].Serialize(), FString(TEXT("lane:b")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRouteInvalidAndBudgetTest,
    "PinkCab.World.Routing.InvalidUnreachableBudget",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRouteInvalidAndBudgetTest::RunTest(const FString& Parameters)
{
    const FPinkCabRoadGraph Graph = BuildTieGraph();
    const FPinkCabCityDeltaState Deltas;
    FPinkCabRoute Route;
    TestFalse(TEXT("missing start lane rejected"),
        FPinkCabRouteService::FindRoute(Graph, Request(TEXT("lane:missing"), TEXT("lane:goal")), Deltas, Route));
    TestFalse(TEXT("bounded search rejects too-small visit budget"),
        FPinkCabRouteService::FindRoute(Graph, Request(TEXT("lane:start"), TEXT("lane:goal"), 2), Deltas, Route));

    FPinkCabRoadGraph Disconnected;
    Disconnected.AddLane(Lane(TEXT("lane:start"), TEXT("n0"), TEXT("n1"), 10.0));
    Disconnected.AddLane(Lane(TEXT("lane:goal"), TEXT("n9"), TEXT("n10"), 10.0));
    TestFalse(TEXT("unreachable goal rejected"),
        FPinkCabRouteService::FindRoute(Disconnected, Request(TEXT("lane:start"), TEXT("lane:goal")), Deltas, Route));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRouteMetadataValidationTest,
    "PinkCab.World.Routing.GraphMetadata",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabRouteMetadataValidationTest::RunTest(const FString& Parameters)
{
    FPinkCabRoadGraph Graph;
    TestFalse(TEXT("zero-length lane rejected"),
        Graph.AddLane(Lane(TEXT("lane:zero"), TEXT("a"), TEXT("b"), 0.0)));
    TestFalse(TEXT("negative layer rejected"),
        Graph.AddLane(Lane(TEXT("lane:bad-layer"), TEXT("a"), TEXT("b"), 10.0, -1)));
    TestTrue(TEXT("positive length and valid layer accepted"),
        Graph.AddLane(Lane(TEXT("lane:ok"), TEXT("a"), TEXT("b"), 10.0, 1)));

    const FPinkCabLogicalLane* Found = Graph.FindLane(FPinkCabLaneId(TEXT("lane:ok")));
    TestNotNull(TEXT("read-only lane lookup available"), Found);
    if (Found)
    {
        TestEqual(TEXT("length metadata preserved"), Found->LengthCm, 10.0);
        TestEqual(TEXT("layer metadata preserved"), Found->Layer, 1);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRouteReconstructionAndReverseTest,
    "PinkCab.World.Routing.ReconstructionAndReverse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabRouteReconstructionAndReverseTest::RunTest(const FString& Parameters)
{
    auto Build = []()
    {
        FPinkCabRoadGraph Graph;
        Graph.AddLane(Lane(TEXT("lane:f0"), TEXT("n0"), TEXT("n1"), 100.0));
        Graph.AddLane(Lane(TEXT("lane:f1"), TEXT("n1"), TEXT("n2"), 100.0));
        Graph.AddLane(Lane(TEXT("lane:r1"), TEXT("n2"), TEXT("n1"), 100.0));
        Graph.AddLane(Lane(TEXT("lane:r0"), TEXT("n1"), TEXT("n0"), 100.0));
        return Graph;
    };

    const FPinkCabCityDeltaState Deltas;
    FPinkCabRoute ForwardA;
    FPinkCabRoute ForwardB;
    const FPinkCabRoadGraph A = Build();
    const FPinkCabRoadGraph B = Build();
    TestTrue(TEXT("forward route A"), FPinkCabRouteService::FindRoute(A, Request(TEXT("lane:f0"), TEXT("lane:f1")), Deltas, ForwardA));
    TestTrue(TEXT("forward route B"), FPinkCabRouteService::FindRoute(B, Request(TEXT("lane:f0"), TEXT("lane:f1")), Deltas, ForwardB));
    TestEqual(TEXT("same reconstruction reproduces route signature"), ForwardA.RouteSignature, ForwardB.RouteSignature);

    FPinkCabRoute Reverse;
    TestTrue(TEXT("graph-valid reverse path resolves"),
        FPinkCabRouteService::FindRoute(A, Request(TEXT("lane:r1"), TEXT("lane:r0")), Deltas, Reverse));
    TestEqual(TEXT("reverse route has two lanes"), Reverse.LaneIds.Num(), 2);
    TestEqual(TEXT("reverse destination lane preserved"), Reverse.LaneIds[1].Serialize(), FString(TEXT("lane:r0")));
    return true;
}

#endif
