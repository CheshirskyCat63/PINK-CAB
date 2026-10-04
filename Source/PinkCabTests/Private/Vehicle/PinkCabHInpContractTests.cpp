#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabHGateGeometry.h"

namespace
{
int32 ResolveGearFromNeutral(const float RightCounts, const float ForwardCounts)
{
    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 0);
    if (!FMath::IsNearlyZero(RightCounts))
    {
        FPinkCabHGateGeometry::ApplyDriverDelta(State, RightCounts, 0.0f);
    }
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
    const bool bReachedNeutral =
        FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -480.0f);
    TestTrue(TEXT("rearward motion from fifth reaches neutral"), bReachedNeutral);
    TestEqual(TEXT("fifth must pass through neutral before reverse"),
        State.RequestedGear, 0);
    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -480.0f);
    TestEqual(TEXT("second rearward phase enters reverse after neutral"),
        State.RequestedGear, -1);

    FPinkCabHGateGeometry::ResetToGear(State, 5);
    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -480.0f);
    const float BeforeX = State.LeverX;
    const float BeforeY = State.LeverY;
    FPinkCabHGateGeometry::ApplyDriverDelta(State, -960.0f, 0.0f);
    TestTrue(TEXT("cross-gate motion remains within authored X bounds"),
        State.LeverX >= -1.0f && State.LeverX <= 2.0f);
    TestTrue(TEXT("cross-gate motion remains within authored Y bounds"),
        State.LeverY >= -1.0f && State.LeverY <= 1.0f);
    TestTrue(TEXT("neutral cross-gate moves without teleporting outside gate"),
        !FMath::IsNearlyEqual(State.LeverX, BeforeX) || !FMath::IsNearlyEqual(State.LeverY, BeforeY));

    return true;
}

#endif
