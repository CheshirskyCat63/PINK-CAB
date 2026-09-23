#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabL1EndlessRoadModel.h"

namespace PinkCabL1EndlessRoadModelTests
{
TArray<int32> LongitudinalCoords(const FPinkCabL1EndlessRoadWindow& Window)
{
    TArray<int32> Result;
    Result.Reserve(Window.DesiredCoords.Num());
    for (const FPinkCabChunkCoord& Coord : Window.DesiredCoords)
    {
        Result.Add(Coord.Longitudinal);
    }
    return Result;
}

void TestWindowCoords(
    FAutomationTestBase& Test,
    const TCHAR* What,
    const FPinkCabL1EndlessRoadWindow& Window,
    std::initializer_list<int32> Expected)
{
    const TArray<int32> Actual = LongitudinalCoords(Window);
    Test.TestEqual(FString::Printf(TEXT("%s count"), What), Actual.Num(), static_cast<int32>(Expected.size()));

    int32 Index = 0;
    for (const int32 ExpectedValue : Expected)
    {
        if (Actual.IsValidIndex(Index))
        {
            Test.TestEqual(
                FString::Printf(TEXT("%s longitudinal[%d]"), What, Index),
                Actual[Index],
                ExpectedValue);
            Test.TestEqual(
                FString::Printf(TEXT("%s lateral[%d]"), What, Index),
                Window.DesiredCoords[Index].Lateral,
                0);
            Test.TestEqual(
                FString::Printf(TEXT("%s layer[%d]"), What, Index),
                Window.DesiredCoords[Index].Layer,
                0);
        }
        ++Index;
    }
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadChunkIndexTest,
    "PinkCab.World.L1EndlessRoad.Model.ChunkIndexBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadChunkIndexTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("origin is chunk zero"),
        FPinkCabL1EndlessRoadModel::ResolveChunkIndex(0.0), 0);
    TestEqual(TEXT("last positive centimetres before seam stay in chunk zero"),
        FPinkCabL1EndlessRoadModel::ResolveChunkIndex(99999.9), 0);
    TestEqual(TEXT("positive seam enters chunk one"),
        FPinkCabL1EndlessRoadModel::ResolveChunkIndex(100000.0), 1);
    TestEqual(TEXT("negative fraction floors into chunk minus one"),
        FPinkCabL1EndlessRoadModel::ResolveChunkIndex(-0.1), -1);
    TestEqual(TEXT("negative exact seam stays chunk minus one"),
        FPinkCabL1EndlessRoadModel::ResolveChunkIndex(-100000.0), -1);
    TestEqual(TEXT("past negative seam floors into chunk minus two"),
        FPinkCabL1EndlessRoadModel::ResolveChunkIndex(-100000.1), -2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadPositiveWindowTest,
    "PinkCab.World.L1EndlessRoad.Model.PositiveTravelWindow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadPositiveWindowTest::RunTest(const FString& Parameters)
{
    const FPinkCabL1EndlessRoadWindow Window =
        FPinkCabL1EndlessRoadModel::BuildWindow(
            10,
            EPinkCabLongitudinalTravelDirection::Positive);

    TestEqual(TEXT("window records current chunk"), Window.CurrentChunkIndex, 10);
    TestEqual(TEXT("window records positive travel"), Window.Direction,
        EPinkCabLongitudinalTravelDirection::Positive);
    PinkCabL1EndlessRoadModelTests::TestWindowCoords(
        *this, TEXT("positive window"), Window, {8, 9, 10, 11, 12, 13, 14});

    TSet<int32> Unique;
    for (const FPinkCabChunkCoord& Coord : Window.DesiredCoords)
    {
        Unique.Add(Coord.Longitudinal);
    }
    TestEqual(TEXT("positive window contains seven unique chunks"), Unique.Num(), 7);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadNegativeWindowTest,
    "PinkCab.World.L1EndlessRoad.Model.NegativeTravelWindow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadNegativeWindowTest::RunTest(const FString& Parameters)
{
    const FPinkCabL1EndlessRoadWindow Window =
        FPinkCabL1EndlessRoadModel::BuildWindow(
            10,
            EPinkCabLongitudinalTravelDirection::Negative);

    TestEqual(TEXT("window records negative travel"), Window.Direction,
        EPinkCabLongitudinalTravelDirection::Negative);
    PinkCabL1EndlessRoadModelTests::TestWindowCoords(
        *this, TEXT("negative window"), Window, {12, 11, 10, 9, 8, 7, 6});
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadDirectionHysteresisTest,
    "PinkCab.World.L1EndlessRoad.Model.DirectionHysteresis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadDirectionHysteresisTest::RunTest(const FString& Parameters)
{
    constexpr double Threshold = 50.0;

    TestEqual(TEXT("positive velocity above threshold selects positive"),
        FPinkCabL1EndlessRoadModel::ResolveTravelDirection(
            51.0, EPinkCabLongitudinalTravelDirection::Negative, Threshold),
        EPinkCabLongitudinalTravelDirection::Positive);

    TestEqual(TEXT("negative velocity below threshold selects negative"),
        FPinkCabL1EndlessRoadModel::ResolveTravelDirection(
            -51.0, EPinkCabLongitudinalTravelDirection::Positive, Threshold),
        EPinkCabLongitudinalTravelDirection::Negative);

    TestEqual(TEXT("near standstill retains prior positive direction"),
        FPinkCabL1EndlessRoadModel::ResolveTravelDirection(
            0.0, EPinkCabLongitudinalTravelDirection::Positive, Threshold),
        EPinkCabLongitudinalTravelDirection::Positive);

    TestEqual(TEXT("small opposite motion does not flap prior negative direction"),
        FPinkCabL1EndlessRoadModel::ResolveTravelDirection(
            25.0, EPinkCabLongitudinalTravelDirection::Negative, Threshold),
        EPinkCabLongitudinalTravelDirection::Negative);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadTopologyTest,
    "PinkCab.World.L1EndlessRoad.Model.StraightChunkTopology",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadTopologyTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City =
        FPinkCabCityIdentity::Create(TEXT("L1-ENDLESS"), TEXT("gen-v1"), TEXT("road-v1"));

    FPinkCabRoadGraph Graph;
    TestTrue(TEXT("one straight chunk appends"),
        FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(City, 0, Graph));
    TestEqual(TEXT("one chunk contributes fourteen ground lanes"),
        Graph.NumLanes(), FPinkCabL1EndlessRoadModel::GroundLaneCount);

    const FPinkCabChunkId Chunk0 = FPinkCabChunkId::From(City, {0, 0, 0});
    const FPinkCabChunkId Chunk1 = FPinkCabChunkId::From(City, {1, 0, 0});

    for (int32 Lane = 0; Lane < 5; ++Lane)
    {
        TestNotNull(TEXT("positive express lane exists"),
            Graph.FindLane(FPinkCabRoadGraph::MakeLaneId(City, Chunk0, 0, Lane)));
        TestNotNull(TEXT("negative express lane exists"),
            Graph.FindLane(FPinkCabRoadGraph::MakeLaneId(City, Chunk0, 1, Lane)));
    }
    for (int32 Lane = 0; Lane < 2; ++Lane)
    {
        TestNotNull(TEXT("positive local lane exists"),
            Graph.FindLane(FPinkCabRoadGraph::MakeLaneId(City, Chunk0, 2, Lane)));
        TestNotNull(TEXT("negative local lane exists"),
            Graph.FindLane(FPinkCabRoadGraph::MakeLaneId(City, Chunk0, 3, Lane)));
    }

    TestNotEqual(TEXT("same lane slot in adjacent chunk has distinct lane id"),
        FPinkCabRoadGraph::MakeLaneId(City, Chunk0, 0, 0).Serialize(),
        FPinkCabRoadGraph::MakeLaneId(City, Chunk1, 0, 0).Serialize());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadSeamConnectivityTest,
    "PinkCab.World.L1EndlessRoad.Model.AdjacentChunkConnectivity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadSeamConnectivityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City =
        FPinkCabCityIdentity::Create(TEXT("L1-ENDLESS"), TEXT("gen-v1"), TEXT("road-v1"));

    FPinkCabRoadGraph Graph;
    TestTrue(TEXT("chunk zero appends"),
        FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(City, 0, Graph));
    TestTrue(TEXT("chunk one appends"),
        FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(City, 1, Graph));
    TestEqual(TEXT("two chunks contribute twenty eight lanes"), Graph.NumLanes(), 28);

    const FPinkCabChunkId Chunk0 = FPinkCabChunkId::From(City, {0, 0, 0});
    const FPinkCabChunkId Chunk1 = FPinkCabChunkId::From(City, {1, 0, 0});

    const FPinkCabLaneId Positive0 = FPinkCabRoadGraph::MakeLaneId(City, Chunk0, 0, 0);
    const FPinkCabLaneId Positive1 = FPinkCabRoadGraph::MakeLaneId(City, Chunk1, 0, 0);
    const TArray<FPinkCabLaneId> PositiveNext = Graph.GetNextLanes(Positive0);
    TestTrue(TEXT("positive carriageway crosses seam into same lane slot"),
        PositiveNext.ContainsByPredicate([&](const FPinkCabLaneId& Id)
        {
            return Id == Positive1;
        }));

    const FPinkCabLaneId Negative1 = FPinkCabRoadGraph::MakeLaneId(City, Chunk1, 1, 0);
    const FPinkCabLaneId Negative0 = FPinkCabRoadGraph::MakeLaneId(City, Chunk0, 1, 0);
    const TArray<FPinkCabLaneId> NegativeNext = Graph.GetNextLanes(Negative1);
    TestTrue(TEXT("negative carriageway crosses seam toward previous chunk"),
        NegativeNext.ContainsByPredicate([&](const FPinkCabLaneId& Id)
        {
            return Id == Negative0;
        }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadTopologyReconstructionTest,
    "PinkCab.World.L1EndlessRoad.Model.TopologyReconstruction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadTopologyReconstructionTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City =
        FPinkCabCityIdentity::Create(TEXT("L1-ENDLESS"), TEXT("gen-v1"), TEXT("road-v1"));

    FPinkCabRoadGraph Forward;
    TestTrue(TEXT("forward chunk zero"), FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(City, 0, Forward));
    TestTrue(TEXT("forward chunk one"), FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(City, 1, Forward));

    FPinkCabRoadGraph ReverseInsertion;
    TestTrue(TEXT("reverse insertion chunk one"), FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(City, 1, ReverseInsertion));
    TestTrue(TEXT("reverse insertion chunk zero"), FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(City, 0, ReverseInsertion));

    TestEqual(TEXT("topology signature ignores chunk insertion order"),
        Forward.GetTopologySignature(),
        ReverseInsertion.GetTopologySignature());
    return true;
}

#endif
