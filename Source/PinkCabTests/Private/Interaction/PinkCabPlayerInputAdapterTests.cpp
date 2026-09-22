#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Interaction/PinkCabPlayerInputAdapter.h"
#include "Interaction/PinkCabSemanticInputRouter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPlayerInputAdapterSemanticSampleTest,
    "PinkCab.Interaction.PlayerInput.SemanticSample",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPlayerInputAdapterSemanticSampleTest::RunTest(const FString& Parameters)
{
    const FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    const TSet<FKey> DownKeys = {
        EKeys::SpaceBar, EKeys::Q, EKeys::W, EKeys::E,
        EKeys::One, EKeys::Three, EKeys::RightMouseButton
    };

    const FPinkCabPlayerInputSample Sample = FPinkCabPlayerInputAdapter::ComposeSample(
        Router,
        [&DownKeys](const FKey& Key) { return DownKeys.Contains(Key); },
        false,
        0.35f,
        -0.20f,
        5.0f,
        -4.0f,
        -1.0f);

    TestTrue(TEXT("Space becomes gaze intent"), Sample.bGazeHeld);
    TestTrue(TEXT("Q clutch coexists with W/E"), Sample.bClutchHeld);
    TestTrue(TEXT("W brake coexists with Q/E"), Sample.bBrakeHeld);
    TestTrue(TEXT("E throttle coexists with Q/W"), Sample.bThrottleHeld);
    TestTrue(TEXT("quick recall 1 is semantic"), Sample.bQuickRecall1Held);
    TestFalse(TEXT("quick recall 2 stays released"), Sample.bQuickRecall2Held);
    TestTrue(TEXT("quick recall 3 is semantic"), Sample.bQuickRecall3Held);
    TestTrue(TEXT("RMB becomes grip"), Sample.bGripHeld);
    TestFalse(TEXT("LMB momentary stays released"), Sample.bMomentaryHeld);
    TestEqual(TEXT("negative wheel becomes one negative step"), Sample.WheelSteps, -1);
    TestEqual(TEXT("processed look X is preserved"), Sample.LookMouseX, 0.35f);
    TestEqual(TEXT("processed look Y is preserved"), Sample.LookMouseY, -0.20f);
    TestEqual(TEXT("active raw X becomes driver-space +X"), Sample.DeviceX, 5.0f);
    TestEqual(TEXT("active raw Y remains canonical device +Y/-Y"), Sample.DeviceY, -4.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPlayerInputAdapterProcessedZeroPreservesRawDeltaTest,
    "PinkCab.Interaction.PlayerInput.ProcessedZeroPreservesRawPhysicalDelta",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPlayerInputAdapterProcessedZeroPreservesRawDeltaTest::RunTest(const FString& Parameters)
{
    const FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    const FPinkCabPlayerInputSample Sample = FPinkCabPlayerInputAdapter::ComposeSample(
        Router,
        [](const FKey&) { return false; },
        false,
        0.0f,
        0.0f,
        19.0f,
        -23.0f,
        0.0f);

    // C-06 regression: processed look input is not a freshness bit for the
    // physical device stream. A valid raw mouse delta must not disappear
    // merely because the processed axis for the same frame is zero.
    TestEqual(TEXT("valid raw X survives a processed-zero frame"), Sample.DeviceX, 19.0f);
    TestEqual(TEXT("valid raw Y survives a processed-zero frame"), Sample.DeviceY, -23.0f);
    TestEqual(TEXT("zero wheel stays zero"), Sample.WheelSteps, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPlayerInputAdapterCanonicalDirectionTest,
    "PinkCab.Interaction.PlayerInput.CanonicalDirections",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPlayerInputAdapterCanonicalDirectionTest::RunTest(const FString& Parameters)
{
    const FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    const FPinkCabPlayerInputSample Sample = FPinkCabPlayerInputAdapter::ComposeSample(
        Router,
        [](const FKey&) { return false; },
        false,
        0.1f,
        0.1f,
        7.0f,
        9.0f,
        1.0f);

    TestEqual(TEXT("positive device X means driver right"), Sample.DeviceX, 7.0f);
    TestEqual(TEXT("positive device Y means gearbox forward/top row"), Sample.DeviceY, 9.0f);
    TestEqual(TEXT("positive wheel becomes one positive step"), Sample.WheelSteps, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPlayerInputAdapterWheelEdgeFallbackTest,
    "PinkCab.Interaction.PlayerInput.WheelEdgeFallback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPlayerInputAdapterWheelEdgeFallbackTest::RunTest(const FString& Parameters)
{
    TestEqual(
        TEXT("scroll-up edge survives a zero packaged analog axis"),
        FPinkCabPlayerInputAdapter::ResolveWheelAxis(0.0f, true, false),
        1.0f);
    TestEqual(
        TEXT("scroll-down edge survives a zero packaged analog axis"),
        FPinkCabPlayerInputAdapter::ResolveWheelAxis(0.0f, false, true),
        -1.0f);
    TestEqual(
        TEXT("analog wheel remains fallback when no discrete edge exists"),
        FPinkCabPlayerInputAdapter::ResolveWheelAxis(-0.5f, false, false),
        -0.5f);
    TestEqual(
        TEXT("contradictory same-frame edges do not invent a direction"),
        FPinkCabPlayerInputAdapter::ResolveWheelAxis(0.0f, true, true),
        0.0f);
    return true;
}

#endif
