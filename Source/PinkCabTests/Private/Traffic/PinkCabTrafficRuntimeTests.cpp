#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Traffic/PinkCabTrafficEntity.h"
#include "Traffic/PinkCabTrafficFlow.h"
#include "Traffic/PinkCabTrafficIncident.h"
#include "Traffic/PinkCabTrafficInteractionBubble.h"
#include "World/PinkCabRouteService.h"

namespace
{
FPinkCabLogicalLane TrafficLane(const TCHAR* Id, const TCHAR* From, const TCHAR* To, double LengthCm)
{
    FPinkCabLogicalLane Lane;
    Lane.LaneId = FPinkCabLaneId(Id);
    Lane.FromNode = FPinkCabRoadNodeId(From);
    Lane.ToNode = FPinkCabRoadNodeId(To);
    Lane.LengthCm = LengthCm;
    Lane.Layer = 0;
    return Lane;
}

FPinkCabRoadGraph BuildTrafficGraph()
{
    FPinkCabRoadGraph Graph;
    Graph.AddLane(TrafficLane(TEXT("lane:0"), TEXT("n0"), TEXT("n1"), 100.0));
    Graph.AddLane(TrafficLane(TEXT("lane:blocked"), TEXT("n1"), TEXT("n2"), 100.0));
    Graph.AddLane(TrafficLane(TEXT("lane:bypass"), TEXT("n1"), TEXT("n3"), 120.0));
    Graph.AddLane(TrafficLane(TEXT("lane:bypass-join"), TEXT("n3"), TEXT("n2"), 120.0));
    Graph.AddLane(TrafficLane(TEXT("lane:goal"), TEXT("n2"), TEXT("n4"), 100.0));
    return Graph;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficLaneTransitionTest,
    "PinkCab.Traffic.Runtime.LaneTransition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficLaneTransitionTest::RunTest(const FString& Parameters)
{
    const FPinkCabRoadGraph Graph = BuildTrafficGraph();
    FPinkCabTrafficEntity Entity(TEXT("traffic-route"), FPinkCabLaneId(TEXT("lane:0")), 80.0, 50.0);
    TArray<FPinkCabLaneId> Route = {FPinkCabLaneId(TEXT("lane:0")), FPinkCabLaneId(TEXT("lane:blocked")), FPinkCabLaneId(TEXT("lane:goal"))};
    TestTrue(TEXT("route assigned"), Entity.TryAssignRoute(Route));
    TestTrue(TEXT("route-aware advance succeeds"), Entity.AdvanceAlongRoute(Graph, 1.0));
    TestEqual(TEXT("entity crosses to next lane"), Entity.GetLaneId().Serialize(), FString(TEXT("lane:blocked")));
    TestEqual(TEXT("overflow distance retained"), Entity.GetLongitudinalCm(), 30.0);
    TestTrue(TEXT("ordinary speed remains positive"), Entity.GetSpeedCmPerSec() > 0.0);
    TestEqual(TEXT("route cursor advanced"), Entity.GetRouteCursor(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficInvalidDeltaTest,
    "PinkCab.Traffic.Runtime.InvalidDelta",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficInvalidDeltaTest::RunTest(const FString& Parameters)
{
    const FPinkCabRoadGraph Graph = BuildTrafficGraph();
    FPinkCabTrafficEntity Entity(TEXT("traffic-delta"), FPinkCabLaneId(TEXT("lane:0")), 10.0, 20.0);
    TestFalse(TEXT("negative delta rejected"), Entity.AdvanceAlongRoute(Graph, -1.0));
    TestEqual(TEXT("position unchanged after invalid delta"), Entity.GetLongitudinalCm(), 10.0);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficIncidentBypassTest,
    "PinkCab.Traffic.Runtime.IncidentBypass",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficIncidentBypassTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("TRAFFIC"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabRoadGraph Graph = BuildTrafficGraph();
    FPinkCabTrafficIncidentRegistry Incidents;
    TestTrue(TEXT("blocked-lane incident registered"),
        Incidents.TryAddBlockedLane(City, FPinkCabLaneId(TEXT("lane:blocked")), TEXT("incident-1")));
    TestTrue(TEXT("registry exposes blocked lane"), Incidents.IsLaneBlocked(FPinkCabLaneId(TEXT("lane:blocked"))));

    FPinkCabTrafficEntity Entity(TEXT("traffic-bypass"), FPinkCabLaneId(TEXT("lane:0")), 100.0, 40.0);
    TArray<FPinkCabLaneId> Original = {FPinkCabLaneId(TEXT("lane:0")), FPinkCabLaneId(TEXT("lane:blocked")), FPinkCabLaneId(TEXT("lane:goal"))};
    TestTrue(TEXT("original route assigned"), Entity.TryAssignRoute(Original));
    const FString StableTrafficId = Entity.GetTrafficId();
    TestTrue(TEXT("reroute only at current lane boundary"),
        FPinkCabTrafficBypassService::TryRerouteAtBoundary(Entity, Graph, Incidents, FPinkCabLaneId(TEXT("lane:goal")), 32));
    TestEqual(TEXT("traffic identity preserved"), Entity.GetTrafficId(), StableTrafficId);
    TestTrue(TEXT("speed remains moving"), Entity.GetSpeedCmPerSec() > 0.0);
    TestTrue(TEXT("advance enters moving bypass"), Entity.AdvanceAlongRoute(Graph, 0.5));
    TestEqual(TEXT("blocked lane avoided"), Entity.GetLaneId().Serialize(), FString(TEXT("lane:bypass")));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficBypassBoundaryGuardTest,
    "PinkCab.Traffic.Runtime.BypassBoundaryGuard",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficBypassBoundaryGuardTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("TRAFFIC"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabRoadGraph Graph = BuildTrafficGraph();
    FPinkCabTrafficIncidentRegistry Incidents;
    TestTrue(TEXT("incident registered"), Incidents.TryAddBlockedLane(
        City, FPinkCabLaneId(TEXT("lane:blocked")), TEXT("incident-guard")));
    FPinkCabTrafficEntity Entity(TEXT("traffic-mid"), FPinkCabLaneId(TEXT("lane:0")), 50.0, 40.0);
    TArray<FPinkCabLaneId> Route = {FPinkCabLaneId(TEXT("lane:0")), FPinkCabLaneId(TEXT("lane:blocked")), FPinkCabLaneId(TEXT("lane:goal"))};
    TestTrue(TEXT("route assigned"), Entity.TryAssignRoute(Route));
    TestFalse(TEXT("mid-lane reroute rejected"),
        FPinkCabTrafficBypassService::TryRerouteAtBoundary(Entity, Graph, Incidents, FPinkCabLaneId(TEXT("lane:goal")), 32));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficInteractionBubbleTest,
    "PinkCab.Traffic.Runtime.InteractionBubble",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficInteractionBubbleTest::RunTest(const FString& Parameters)
{
    FPinkCabTrafficInteractionBubble Bubble(2);
    TArray<FPinkCabTrafficProxyCandidate> First = {{TEXT("a"), 100.0}, {TEXT("b"), 25.0}, {TEXT("c"), 400.0}};
    FPinkCabTrafficInteractionBubbleResult ResultA;
    TestTrue(TEXT("first bubble update"), Bubble.Update(First, ResultA));
    TestEqual(TEXT("hard proxy cap enforced"), ResultA.Assignments.Num(), 2);
    TestEqual(TEXT("nearest candidate promoted first"), ResultA.Assignments[0].TrafficId, FString(TEXT("b")));
    const int32 SlotB = ResultA.Assignments[0].SlotIndex;
    TArray<FPinkCabTrafficProxyCandidate> Second = {{TEXT("b"), 16.0}, {TEXT("d"), 9.0}};
    FPinkCabTrafficInteractionBubbleResult ResultB;
    TestTrue(TEXT("second bubble update"), Bubble.Update(Second, ResultB));
    TestEqual(TEXT("capacity remains bounded"), ResultB.Assignments.Num(), 2);
    bool bBKeptSlot = false;
    for (const FPinkCabTrafficProxyAssignment& Assignment : ResultB.Assignments)
    {
        if (Assignment.TrafficId == TEXT("b"))
        {
            bBKeptSlot = Assignment.SlotIndex == SlotB;
        }
    }
    TestTrue(TEXT("retained traffic keeps stable slot"), bBKeptSlot);
    TestEqual(TEXT("one leaving id demoted"), ResultB.DemotedTrafficIds.Num(), 1);
    TestEqual(TEXT("leaving id reported"), ResultB.DemotedTrafficIds[0], FString(TEXT("a")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTrafficInteractionBubbleDuplicateTest,
    "PinkCab.Traffic.Runtime.InteractionBubbleDuplicateIds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTrafficInteractionBubbleDuplicateTest::RunTest(const FString& Parameters)
{
    FPinkCabTrafficInteractionBubble Bubble(2);
    TArray<FPinkCabTrafficProxyCandidate> Duplicates = {{TEXT("same"), 1.0}, {TEXT("same"), 4.0}};
    FPinkCabTrafficInteractionBubbleResult Result;
    TestFalse(TEXT("duplicate candidate ids rejected"), Bubble.Update(Duplicates, Result));
    TestEqual(TEXT("failed update owns no proxies"), Bubble.NumAssigned(), 0);
    return true;
}

#endif
