#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabL1RoadConstructionModel.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadR2ConstructionModelTest,
    "PinkCab.World.L1Road.R2.ConstructionModel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadR2ConstructionModelTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("R2 central median remains 8m wide"),
        FPinkCabL1RoadConstructionModel::CentralMedianWidthCm, 800.0);
    TestEqual(TEXT("R2 road half-envelope remains frozen at 33.4m"),
        FPinkCabL1RoadConstructionModel::RoadHalfWidthCm, 3340.0);
    TestEqual(TEXT("express outer edge is frozen"),
        FPinkCabL1RoadConstructionModel::ExpressOuterEdgeAbsYCm, 2200.0);
    TestEqual(TEXT("service-band outer edge is frozen"),
        FPinkCabL1RoadConstructionModel::ServiceOuterEdgeAbsYCm, 2600.0);
    TestEqual(TEXT("local-road outer edge is frozen"),
        FPinkCabL1RoadConstructionModel::LocalOuterEdgeAbsYCm, 3240.0);

    TestTrue(TEXT("median deck is visibly raised"),
        FPinkCabL1RoadConstructionModel::MedianDeckHeightCm >= 6.0);
    TestTrue(TEXT("curb is higher than median deck"),
        FPinkCabL1RoadConstructionModel::CurbHeightCm >
            FPinkCabL1RoadConstructionModel::MedianDeckHeightCm);
    TestTrue(TEXT("curb stays low enough to read as road construction"),
        FPinkCabL1RoadConstructionModel::CurbHeightCm <= 14.0);
    TestTrue(TEXT("curb has readable width"),
        FPinkCabL1RoadConstructionModel::CurbWidthCm >= 16.0);

    const auto Closed =
        FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(0.0);
    TestTrue(TEXT("service separator exists on straight"),
        Closed.bPresent);
    TestEqual(TEXT("straight service separator is full 4m"),
        Closed.WidthCm, 400.0);
    TestEqual(TEXT("straight service separator center is 24m"),
        Closed.CenterAbsYCm, 2400.0);

    const auto ExitThroat =
        FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(30000.0);
    TestFalse(TEXT("R1 exit throat remains physically open"),
        ExitThroat.bPresent);
    TestEqual(TEXT("R1 exit throat separator width is zero"),
        ExitThroat.WidthCm, 0.0);

    const auto MidStraight =
        FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(50000.0);
    TestTrue(TEXT("separator returns between R1 windows"),
        MidStraight.bPresent);
    TestEqual(TEXT("middle straight separator returns to 4m"),
        MidStraight.WidthCm, 400.0);

    const auto MergeThroat =
        FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(70000.0);
    TestFalse(TEXT("R1 merge throat remains physically open"),
        MergeThroat.bPresent);
    TestEqual(TEXT("R1 merge throat separator width is zero"),
        MergeThroat.WidthCm, 0.0);

    const auto Taper =
        FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(21250.0);
    TestTrue(TEXT("separator remains present through taper"),
        Taper.bPresent);
    TestTrue(TEXT("taper separator is approximately 2m wide"),
        FMath::IsNearlyEqual(Taper.WidthCm, 200.0, 0.01));
    TestTrue(TEXT("taper stays pinned to local-road edge"),
        FMath::IsNearlyEqual(
            Taper.CenterAbsYCm + Taper.WidthCm * 0.5,
            FPinkCabL1RoadConstructionModel::ServiceOuterEdgeAbsYCm,
            0.01));

    const double OuterCurbCenter =
        FPinkCabL1RoadConstructionModel::ResolveOuterCurbCenterAbsYCm();
    TestTrue(TEXT("outer curb remains inside frozen envelope"),
        OuterCurbCenter +
            FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5 <=
            FPinkCabL1RoadConstructionModel::RoadHalfWidthCm + 0.01);
    TestTrue(TEXT("outer curb sits beyond local carriageway"),
        OuterCurbCenter >
            FPinkCabL1RoadConstructionModel::LocalOuterEdgeAbsYCm);

    TestEqual(TEXT("R2 construction slice is 5m"),
        FPinkCabL1RoadConstructionModel::ConstructionSliceLengthCm, 500.0);
    TestEqual(TEXT("1000m module slices exactly"),
        static_cast<int32>(
            FPinkCabL1RoadConstructionModel::ChunkLengthCm /
            FPinkCabL1RoadConstructionModel::ConstructionSliceLengthCm),
        200);

    return true;
}

#endif
