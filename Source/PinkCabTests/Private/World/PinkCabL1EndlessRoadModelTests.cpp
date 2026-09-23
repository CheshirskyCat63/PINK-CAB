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

#endif
