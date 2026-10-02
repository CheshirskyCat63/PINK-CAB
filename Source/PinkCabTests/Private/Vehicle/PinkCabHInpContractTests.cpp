#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabHGateGeometry.h"

namespace
{
int32 ResolveGearFromNeutral(const float RightCounts, const float ForwardCounts)
{
    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 0);
    // Accepted H-gate input has distinct cross-gate and fore/aft phases.
    FPinkCabHGateGeometry::ApplyDriverDelta(State, RightCounts, 0.0f);
    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, ForwardCounts);
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
        ResolveGearFromNeutral(-640.0f, 480.0f), 1);
    TestEqual(TEXT("top-center is third"),
        ResolveGearFromNeutral(0.0f, 480.0f), 3);
    TestEqual(TEXT("top-right is fifth"),
        ResolveGearFromNeutral(320.0f, 480.0f), 5);

    TestEqual(TEXT("bottom-left is second"),
        ResolveGearFromNeutral(-640.0f, -480.0f), 2);
    TestEqual(TEXT("bottom-center is fourth"),
        ResolveGearFromNeutral(0.0f, -480.0f), 4);
    TestEqual(TEXT("bottom-right is reverse"),
        ResolveGearFromNeutral(320.0f, -480.0f), -1);

    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 5);
    const bool bDiagonalMoved =
        FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -960.0f);
    TestTrue(TEXT("large vertical motion from fifth traverses neutral"), bDiagonalMoved);
    TestEqual(TEXT("one large throw from fifth stops in neutral"),
        State.RequestedGear, 0);
    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -480.0f);
    TestEqual(TEXT("a separate rearward throw enters reverse"),
        State.RequestedGear, -1);

    FPinkCabHGateGeometry::ResetToGear(State, 5);
    const float BeforeX = State.LeverX;
    const float BeforeY = State.LeverY;
    FPinkCabHGateGeometry::ApplyDriverDelta(State, -960.0f, -960.0f);
    TestTrue(TEXT("cross-gate motion remains within authored X bounds"),
        State.LeverX >= -1.0f && State.LeverX <= 2.0f);
    TestTrue(TEXT("cross-gate motion remains within authored Y bounds"),
        State.LeverY >= -1.0f && State.LeverY <= 1.0f);
    TestTrue(TEXT("cross-gate motion changes from fifth without teleporting outside gate"),
        !FMath::IsNearlyEqual(State.LeverX, BeforeX) || !FMath::IsNearlyEqual(State.LeverY, BeforeY));

    TestEqual(TEXT("diagonal exit cannot cross columns in the same sample"),
        State.LeverX, BeforeX);
    TestEqual(TEXT("diagonal exit stops at neutral"), State.RequestedGear, 0);
    return true;
}

#endif
