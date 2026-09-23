#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabHGateGeometry.h"

namespace
{
int32 ResolveGearFromNeutral(const float RightCounts, const float ForwardCounts)
{
    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 0);
    FPinkCabHGateGeometry::ApplyDriverDelta(State, RightCounts, ForwardCounts);
    return State.RequestedGear;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabHInpHGateLayoutTest,
    "PinkCab.G1.HInp.HGate.Layout",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabHInpHGateLayoutTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("top-left is first"),
        ResolveGearFromNeutral(-160.0f, 140.0f), 1);
    TestEqual(TEXT("top-center is third"),
        ResolveGearFromNeutral(0.0f, 140.0f), 3);
    TestEqual(TEXT("top-right is fifth"),
        ResolveGearFromNeutral(160.0f, 140.0f), 5);

    TestEqual(TEXT("bottom-left is second"),
        ResolveGearFromNeutral(-160.0f, -140.0f), 2);
    TestEqual(TEXT("bottom-center is fourth"),
        ResolveGearFromNeutral(0.0f, -140.0f), 4);
    TestEqual(TEXT("bottom-right is reverse"),
        ResolveGearFromNeutral(160.0f, -140.0f), -1);

    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 5);
    const bool bDiagonalMoved =
        FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -280.0f);
    TestTrue(TEXT("large vertical motion from fifth traverses neutral"), bDiagonalMoved);
    TestEqual(TEXT("fifth-to-reverse ends in reverse only through neutral"),
        State.RequestedGear, -1);

    FPinkCabHGateGeometry::ResetToGear(State, 5);
    const float BeforeX = State.LeverX;
    const float BeforeY = State.LeverY;
    FPinkCabHGateGeometry::ApplyDriverDelta(State, -320.0f, -280.0f);
    TestTrue(TEXT("cross-gate motion remains within authored X bounds"),
        State.LeverX >= -1.0f && State.LeverX <= 1.0f);
    TestTrue(TEXT("cross-gate motion remains within authored Y bounds"),
        State.LeverY >= -1.0f && State.LeverY <= 1.0f);
    TestTrue(TEXT("cross-gate motion changes from fifth without teleporting outside gate"),
        !FMath::IsNearlyEqual(State.LeverX, BeforeX) || !FMath::IsNearlyEqual(State.LeverY, BeforeY));

    return true;
}

#endif
