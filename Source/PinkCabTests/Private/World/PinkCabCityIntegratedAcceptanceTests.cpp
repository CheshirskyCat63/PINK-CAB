#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Enforcement/PinkCabEnforcementService.h"
#include "Traffic/PinkCabTrafficFlow.h"
#include "Traffic/PinkCabTrafficIncident.h"
#include "Traffic/PinkCabTrafficInteractionBubble.h"
#include "World/PinkCabCityDeltaState.h"
#include "World/PinkCabCityLocationRegistry.h"
#include "World/PinkCabRouteService.h"
#include "World/PinkCabWorldMaterializationPolicy.h"

namespace
{
struct FIntegratedCityFixture
{
    FPinkCabCityIdentity City;
    FPinkCabChunkId L1Chunk;
    FPinkCabChunkId L2Chunk;
    FPinkCabLaneId StartLane;
    FPinkCabLaneId BlockedLane;
    FPinkCabLaneId BypassLane;
    FPinkCabLaneId BypassJoinLane;
    FPinkCabLaneId GoalLane;
    FPinkCabLaneId L2ServiceLane;
    FPinkCabRoadGraph Graph;
    FPinkCabCityDeltaState Deltas;
    FPinkCabCityLocationRegistry Anchors;
    FString RuleAnchorId;
    bool Build()
    {
        City = FPinkCabCityIdentity::Create(TEXT("ACCEPT"), TEXT("gen-1"), TEXT("content-1"));
        L1Chunk = FPinkCabChunkId::From(City, {0, 0, 0});
        L2Chunk = FPinkCabChunkId::From(City, {0, 0, 1});
        StartLane = FPinkCabRoadGraph::MakeLaneId(City, L1Chunk, 0, 0);
        BlockedLane = FPinkCabRoadGraph::MakeLaneId(City, L1Chunk, 1, 0);
        BypassLane = FPinkCabRoadGraph::MakeLaneId(City, L1Chunk, 2, 0);
        BypassJoinLane = FPinkCabRoadGraph::MakeLaneId(City, L1Chunk, 3, 0);
        GoalLane = FPinkCabRoadGraph::MakeLaneId(City, L1Chunk, 4, 0);
        L2ServiceLane = FPinkCabRoadGraph::MakeLaneId(City, L2Chunk, 0, 0);

        const FPinkCabRoadNodeId N0 = FPinkCabRoadGraph::MakeNodeId(City, L1Chunk, 0);
        const FPinkCabRoadNodeId N1 = FPinkCabRoadGraph::MakeNodeId(City, L1Chunk, 1);
        const FPinkCabRoadNodeId N2 = FPinkCabRoadGraph::MakeNodeId(City, L1Chunk, 2);
        const FPinkCabRoadNodeId N3 = FPinkCabRoadGraph::MakeNodeId(City, L1Chunk, 3);
        const FPinkCabRoadNodeId N4 = FPinkCabRoadGraph::MakeNodeId(City, L1Chunk, 4);
        const FPinkCabRoadNodeId M0 = FPinkCabRoadGraph::MakeNodeId(City, L2Chunk, 0);
        const FPinkCabRoadNodeId M1 = FPinkCabRoadGraph::MakeNodeId(City, L2Chunk, 1);

        return AddLane(StartLane, N0, N1, 100.0, 0)
            && AddLane(BlockedLane, N1, N2, 100.0, 0)
            && AddLane(BypassLane, N1, N3, 120.0, 0)
            && AddLane(BypassJoinLane, N3, N2, 120.0, 0)
            && AddLane(GoalLane, N2, N4, 100.0, 0)
            && AddLane(L2ServiceLane, M0, M1, 150.0, 1)
            && AddPersistentState();
    }

    bool AddLane(
        const FPinkCabLaneId& LaneId,
        const FPinkCabRoadNodeId& From,
        const FPinkCabRoadNodeId& To,
        double LengthCm,
        int32 Layer)
    {
        FPinkCabLogicalLane Lane;
        Lane.LaneId = LaneId;
        Lane.FromNode = From;
        Lane.ToNode = To;
        Lane.LengthCm = LengthCm;
        Lane.Layer = Layer;
        return Graph.AddLane(Lane);
    }

    bool AddPersistentState()
    {
        FString DeltaId;
        if (!Deltas.TryAddLaneClosure(City, BlockedLane, TEXT("acceptance-incident"), DeltaId))
        {
            return false;
        }
        if (!AddAnchor(StartLane, EPinkCabCityLocationKind::Pickup, TEXT("pickup"), 0, 10.0)) return false;
        if (!AddAnchor(GoalLane, EPinkCabCityLocationKind::Destination, TEXT("destination"), 0, 90.0)) return false;
        if (!AddAnchor(L2ServiceLane, EPinkCabCityLocationKind::Service, TEXT("service"), 1, 40.0)) return false;
        FPinkCabCityLocationAnchor RuleAnchor;
        RuleAnchor.AnchorId = FPinkCabCityLocationRegistry::MakeAnchorId(
            City, BypassLane, EPinkCabCityLocationKind::Rule, TEXT("speed-rule"));
        RuleAnchor.Kind = EPinkCabCityLocationKind::Rule;
        RuleAnchor.LaneId = BypassLane;
        RuleAnchor.LongitudinalCm = 60.0;
        RuleAnchor.Layer = 0;
        RuleAnchorId = RuleAnchor.AnchorId;
        return Anchors.TryRegister(RuleAnchor);
    }

    bool AddAnchor(
        const FPinkCabLaneId& LaneId,
        EPinkCabCityLocationKind Kind,
        const TCHAR* Key,
        int32 Layer,
        double LongitudinalCm)
    {
        FPinkCabCityLocationAnchor Anchor;
        Anchor.AnchorId = FPinkCabCityLocationRegistry::MakeAnchorId(City, LaneId, Kind, Key);
        Anchor.Kind = Kind;
        Anchor.LaneId = LaneId;
        Anchor.LongitudinalCm = LongitudinalCm;
        Anchor.Layer = Layer;
        return Anchors.TryRegister(Anchor);
    }
};

FPinkCabWorldMaterializationSettings AcceptanceMaterializationSettings()
{
    FPinkCabWorldMaterializationSettings Settings;
    Settings.MaxMaterializedChunks = 2;
    Settings.MaxDistanceSquared = 9;
    Settings.AllowedLayers = {0, 1};
    Settings.MaxAnchorRequests = 4;
    return Settings;
}

TArray<FPinkCabWorldChunkCandidate> AcceptanceCandidates(const FPinkCabCityIdentity& City)
{
    TArray<FPinkCabWorldChunkCandidate> Result;
    auto Add = [&](int32 X, int32 Layer)
    {
        FPinkCabWorldChunkCandidate Candidate;
        Candidate.Coord = {X, 0, Layer};
        Candidate.ChunkId = FPinkCabChunkId::From(City, Candidate.Coord);
        Result.Add(Candidate);
    };
    Add(0, 0); Add(0, 1); Add(2, 0); Add(2, 1); Add(4, 0);
    return Result;
}
} // namespace
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityIntegratedReconstructionTest,
    "PinkCab.World.CityRuntime.Acceptance.DeterministicReconstruction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityIntegratedReconstructionTest::RunTest(const FString& Parameters)
{
    FIntegratedCityFixture A;
    FIntegratedCityFixture B;
    TestTrue(TEXT("fixture A builds"), A.Build());
    TestTrue(TEXT("fixture B builds"), B.Build());
    TestEqual(TEXT("topology reconstructs"), A.Graph.GetTopologySignature(), B.Graph.GetTopologySignature());
    TestEqual(TEXT("persistent deltas reconstruct"), A.Deltas.GetReconstructionSignature(), B.Deltas.GetReconstructionSignature());
    TestEqual(TEXT("anchors reconstruct"), A.Anchors.GetReconstructionSignature(), B.Anchors.GetReconstructionSignature());

    FPinkCabRouteRequest Request;
    Request.StartLaneId = A.StartLane;
    Request.GoalLaneId = A.GoalLane;
    Request.MaxVisitedNodes = 32;
    FPinkCabRoute RouteA;
    FPinkCabRoute RouteB;
    TestTrue(TEXT("route A resolves"), FPinkCabRouteService::FindRoute(A.Graph, Request, A.Deltas, RouteA));
    Request.StartLaneId = B.StartLane;
    Request.GoalLaneId = B.GoalLane;
    TestTrue(TEXT("route B resolves"), FPinkCabRouteService::FindRoute(B.Graph, Request, B.Deltas, RouteB));
    TestEqual(TEXT("route signature reconstructs"), RouteA.RouteSignature, RouteB.RouteSignature);

    const TArray<FPinkCabWorldChunkCandidate> Candidates = AcceptanceCandidates(A.City);
    const FPinkCabWorldMaterializationSettings Settings = AcceptanceMaterializationSettings();
    FPinkCabWorldMaterializationResult Start;
    FPinkCabWorldMaterializationResult Forward;
    FPinkCabWorldMaterializationResult Return;
    TestTrue(TEXT("start materialization"), FPinkCabWorldMaterializationPolicy::BuildWindow(
        {0, 0, 0}, Candidates, Settings, {}, Start));
    TestTrue(TEXT("forward materialization"), FPinkCabWorldMaterializationPolicy::BuildWindow(
        {2, 0, 0}, Candidates, Settings, Start.MaterializeChunkIds, Forward));
    TestTrue(TEXT("return materialization"), FPinkCabWorldMaterializationPolicy::BuildWindow(
        {0, 0, 0}, Candidates, Settings, Forward.MaterializeChunkIds, Return));
    TestEqual(TEXT("return reproduces materialization signature"), Start.RequestSignature, Return.RequestSignature);
    TestTrue(TEXT("materialization remains bounded"), Return.MaterializeChunkIds.Num() <= Settings.MaxMaterializedChunks);

    TArray<FString> AnchorRequests;
    TestTrue(TEXT("graph anchors materialize"), FPinkCabWorldMaterializationPolicy::BuildAnchorRequests(
        A.Anchors, Settings, AnchorRequests));
    TestEqual(TEXT("pickup destination service rule anchors all requested"), AnchorRequests.Num(), 4);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityIntegratedRuntimeTest,
    "PinkCab.World.CityRuntime.Acceptance.MovingTrafficEnforcement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityIntegratedRuntimeTest::RunTest(const FString& Parameters)
{
    FIntegratedCityFixture Fixture;
    TestTrue(TEXT("fixture builds"), Fixture.Build());
    FPinkCabRouteRequest RouteRequest;
    RouteRequest.StartLaneId = Fixture.StartLane;
    RouteRequest.GoalLaneId = Fixture.GoalLane;
    RouteRequest.MaxVisitedNodes = 32;
    FPinkCabRoute MovingRoute;
    TestTrue(TEXT("moving route resolves around persistent closure"),
        FPinkCabRouteService::FindRoute(Fixture.Graph, RouteRequest, Fixture.Deltas, MovingRoute));

    FPinkCabTrafficFlowConstraints FlowConstraints;
    FlowConstraints.MaxLogicalEntities = 2;
    FlowConstraints.MinPlayableGapCm = 25.0;
    FPinkCabTrafficFlow Flow(FlowConstraints);
    FPinkCabTrafficEntity Lead(TEXT("accept-lead"), Fixture.StartLane, 90.0, 40.0);
    FPinkCabTrafficEntity Follow(TEXT("accept-follow"), Fixture.StartLane, 20.0, 30.0);
    TestTrue(TEXT("lead route assigned"), Lead.TryAssignRoute(MovingRoute.LaneIds));
    TestTrue(TEXT("follow route assigned"), Follow.TryAssignRoute(MovingRoute.LaneIds));
    TestTrue(TEXT("lead accepted"), Flow.TryAddOrdinary(Lead));
    TestTrue(TEXT("follow accepted"), Flow.TryAddOrdinary(Follow));
    TestFalse(TEXT("logical traffic ceiling enforced"), Flow.TryAddOrdinary(
        FPinkCabTrafficEntity(TEXT("overflow"), Fixture.StartLane, 60.0, 20.0)));
    TestTrue(TEXT("route-aware flow advances"), Flow.AdvanceAllAlongRoutes(Fixture.Graph, 1.0));
    FPinkCabTrafficEntity LeadAfter;
    TestTrue(TEXT("lead remains queryable"), Flow.TryGetEntity(TEXT("accept-lead"), LeadAfter));
    TestEqual(TEXT("lead enters deterministic bypass"),
        LeadAfter.GetLaneId().Serialize(), Fixture.BypassLane.Serialize());
    TestTrue(TEXT("ordinary traffic remains moving"), LeadAfter.GetSpeedCmPerSec() > 0.0);

    FPinkCabTrafficInteractionBubble Bubble(2);
    FPinkCabTrafficInteractionBubbleResult BubbleResult;
    TArray<FPinkCabTrafficProxyCandidate> ProxyCandidates = {
        {TEXT("accept-lead"), 4.0}, {TEXT("accept-follow"), 9.0}, {TEXT("far-a"), 25.0}};
    TestTrue(TEXT("proxy bubble updates"), Bubble.Update(ProxyCandidates, BubbleResult));
    TestEqual(TEXT("promoted proxy cap enforced"), BubbleResult.Assignments.Num(), 2);

    FPinkCabTrafficIncidentRegistry Incidents;
    TestTrue(TEXT("acceptance incident registered"), Incidents.TryAddBlockedLane(
        Fixture.City, Fixture.BlockedLane, TEXT("acceptance-runtime")));
    FPinkCabTrafficEntity Boundary(TEXT("accept-boundary"), Fixture.StartLane, 100.0, 20.0);
    TArray<FPinkCabLaneId> OriginalRoute = {Fixture.StartLane, Fixture.BlockedLane, Fixture.GoalLane};
    TestTrue(TEXT("original route assigned"), Boundary.TryAssignRoute(OriginalRoute));
    TestTrue(TEXT("boundary incident reroutes"), FPinkCabTrafficBypassService::TryRerouteAtBoundary(
        Boundary, Fixture.Graph, Incidents, Fixture.GoalLane, 32));
    TestTrue(TEXT("rerouted entity keeps moving"), Boundary.AdvanceAlongRoute(Fixture.Graph, 0.5));
    TestEqual(TEXT("incident bypass avoids blocked lane"),
        Boundary.GetLaneId().Serialize(), Fixture.BypassLane.Serialize());

    FPinkCabRoadRuleProfile Rules;
    FPinkCabRoadRuleDefinition SpeedRule;
    SpeedRule.Kind = EPinkCabRoadRuleKind::SpeedLimit;
    SpeedRule.TypeId = FName(TEXT("AcceptanceSpeed"));
    SpeedRule.SpeedLimitCmPerSec = 100.0;
    SpeedRule.FineAmountMinor = 1000;
    SpeedRule.ReputationDelta = -4;
    SpeedRule.BaseSeverity = 0.5f;
    TestTrue(TEXT("acceptance speed rule registered"), Rules.TrySetRule(Fixture.BypassLane, SpeedRule));

    auto MakeObservation = [&](double Speed)
    {
        FPinkCabRoadObservation Observation;
        Observation.ObservationKey = TEXT("acceptance-route-sample");
        Observation.LaneId = Fixture.BypassLane;
        Observation.CityLocationId = FPinkCabStableId(Fixture.RuleAnchorId);
        Observation.VehicleId = FPinkCabStableId(TEXT("vehicle:acceptance-hero"));
        Observation.TimestampMs = 777;
        Observation.SpeedCmPerSec = Speed;
        return Observation;
    };
    TArray<FPinkCabEnforcementEvent> CalmEvents;
    TArray<FPinkCabEnforcementEvent> RecklessA;
    TArray<FPinkCabEnforcementEvent> RecklessB;
    TestTrue(TEXT("calm traversal evaluates"),
        FPinkCabEnforcementService::Evaluate(Rules, MakeObservation(90.0), CalmEvents));
    TestEqual(TEXT("calm traversal emits no speed event"), CalmEvents.Num(), 0);
    TestTrue(TEXT("reckless traversal evaluates"),
        FPinkCabEnforcementService::Evaluate(Rules, MakeObservation(160.0), RecklessA));
    TestTrue(TEXT("reckless traversal repeat evaluates"),
        FPinkCabEnforcementService::Evaluate(Rules, MakeObservation(160.0), RecklessB));
    TestEqual(TEXT("reckless traversal emits one event"), RecklessA.Num(), 1);
    TestEqual(TEXT("reckless event id repeats"),
        RecklessA[0].EventId.Serialize(), RecklessB[0].EventId.Serialize());

    FPinkCabEconomyLedger Economy(5000, 2000);
    FPinkCabEnforcementLedger Reputation(8);
    FPinkCabEnforcementSettlement FirstSettlement;
    TestEqual(TEXT("first fine commits"), FPinkCabEnforcementService::Settle(
        Rules, RecklessA[0], Economy, Reputation, FirstSettlement),
        EPinkCabEnforcementSettlementResult::Committed);
    TestEqual(TEXT("fine debits once"), Economy.GetBalanceMinor(), int64(4000));
    TestEqual(TEXT("city reputation applies once"), Reputation.GetReputationScore(), -4);
    FPinkCabEnforcementSettlement ReplaySettlement;
    TestEqual(TEXT("replayed fine rejected as duplicate"), FPinkCabEnforcementService::Settle(
        Rules, RecklessB[0], Economy, Reputation, ReplaySettlement),
        EPinkCabEnforcementSettlementResult::Duplicate);
    TestEqual(TEXT("duplicate leaves economy unchanged"), Economy.GetBalanceMinor(), int64(4000));
    TestEqual(TEXT("duplicate leaves reputation unchanged"), Reputation.GetReputationScore(), -4);
    TestEqual(TEXT("fine transaction id remains stable"),
        FirstSettlement.TransactionId.GetValue(), ReplaySettlement.TransactionId.GetValue());
    return true;
}

#endif
