#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabCityIdentity.h"
#include "World/PinkCabChunkId.h"
#include "World/PinkCabRoadGraph.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityIdentityTest,
    "PinkCab.World.CityCode.Identity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityIdentityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity A = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabCityIdentity B = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    TestTrue(TEXT("identity valid"), A.IsValid());
    TestEqual(TEXT("same inputs same stable key"), A.GetStableKey(), B.GetStableKey());
    TestTrue(TEXT("same CityCode is same campaign city"), A.IsSameCampaignCity(B));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityVersionSensitivityTest,
    "PinkCab.World.CityCode.VersionSensitivity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityVersionSensitivityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity Base = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabCityIdentity NewGenerator = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-2"), TEXT("content-1"));
    const FPinkCabCityIdentity OtherCity = FPinkCabCityIdentity::Create(TEXT("EURO-64"), TEXT("gen-1"), TEXT("content-1"));
    TestNotEqual(TEXT("generator version changes reconstruction key"), Base.GetStableKey(), NewGenerator.GetStableKey());
    TestTrue(TEXT("same CityCode still names same campaign city"), Base.IsSameCampaignCity(NewGenerator));
    TestFalse(TEXT("different CityCode is a different campaign city"), Base.IsSameCampaignCity(OtherCity));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChunkIdTest,
    "PinkCab.World.CityCode.ChunkIds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabChunkIdTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabChunkCoord Coord{12, 0, 1};
    const FPinkCabChunkId A = FPinkCabChunkId::From(City, Coord);
    const FPinkCabChunkId B = FPinkCabChunkId::From(City, Coord);
    const FPinkCabChunkId Other = FPinkCabChunkId::From(City, FPinkCabChunkCoord{13, 0, 1});
    TestEqual(TEXT("same city/coord same chunk id"), A.Serialize(), B.Serialize());
    TestNotEqual(TEXT("different coord different chunk id"), A.Serialize(), Other.Serialize());
    TestTrue(TEXT("chunk id valid"), A.IsValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRoadLaneIdTest,
    "PinkCab.World.CityCode.RoadLaneIds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRoadLaneIdTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabChunkId Chunk = FPinkCabChunkId::From(City, {2, 0, 0});
    TestEqual(TEXT("road node deterministic"), FPinkCabRoadGraph::MakeNodeId(City, Chunk, 4).Serialize(), FPinkCabRoadGraph::MakeNodeId(City, Chunk, 4).Serialize());
    TestEqual(TEXT("lane deterministic"), FPinkCabRoadGraph::MakeLaneId(City, Chunk, 1, 3).Serialize(), FPinkCabRoadGraph::MakeLaneId(City, Chunk, 1, 3).Serialize());
    TestNotEqual(TEXT("different lane index different id"), FPinkCabRoadGraph::MakeLaneId(City, Chunk, 1, 3).Serialize(), FPinkCabRoadGraph::MakeLaneId(City, Chunk, 1, 4).Serialize());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRoadGraphAdjacencyTest,
    "PinkCab.World.CityCode.UnloadedAdjacency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRoadGraphAdjacencyTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabChunkId Chunk = FPinkCabChunkId::From(City, {0, 0, 0});
    const FPinkCabRoadNodeId N0 = FPinkCabRoadGraph::MakeNodeId(City, Chunk, 0);
    const FPinkCabRoadNodeId N1 = FPinkCabRoadGraph::MakeNodeId(City, Chunk, 1);
    const FPinkCabRoadNodeId N2 = FPinkCabRoadGraph::MakeNodeId(City, Chunk, 2);
    const FPinkCabLaneId L0 = FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, 0);
    const FPinkCabLaneId L1 = FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, 1);

    FPinkCabRoadGraph Graph;
    TestTrue(TEXT("first logical lane added"), Graph.AddLane({L0, N0, N1}));
    TestTrue(TEXT("second logical lane added"), Graph.AddLane({L1, N1, N2}));
    const TArray<FPinkCabLaneId> Next = Graph.GetNextLanes(L0);
    TestEqual(TEXT("pure data graph resolves adjacency without materialized actors"), Next.Num(), 1);
    TestEqual(TEXT("next logical lane resolved"), Next[0].Serialize(), L1.Serialize());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRoadGraphReproductionTest,
    "PinkCab.World.CityCode.RepeatedTopology",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRoadGraphReproductionTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabChunkId Chunk = FPinkCabChunkId::From(City, {7, 0, 0});
    auto Build = [&]()
    {
        FPinkCabRoadGraph Graph;
        const auto N0 = FPinkCabRoadGraph::MakeNodeId(City, Chunk, 0);
        const auto N1 = FPinkCabRoadGraph::MakeNodeId(City, Chunk, 1);
        const auto N2 = FPinkCabRoadGraph::MakeNodeId(City, Chunk, 2);
        Graph.AddLane({FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, 0), N0, N1});
        Graph.AddLane({FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, 1), N1, N2});
        return Graph;
    };
    TestEqual(TEXT("repeated inputs reproduce topology signature"), Build().GetTopologySignature(), Build().GetTopologySignature());
    return true;
}

#endif
