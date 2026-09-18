#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"
#include "Vehicle/PinkCabLaunchController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleMotionHysteresisTest,
    "PinkCab.Vehicle.ControlRuntime.Motion.Hysteresis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleMotionHysteresisTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleMotionClassifier Motion;
    TestEqual(TEXT("starts stationary"), Motion.GetMode(), EPinkCabVehicleMotionMode::Stationary);

    Motion.Update(2.0f, 0.05f);
    TestEqual(TEXT("moving threshold is debounced"), Motion.GetMode(), EPinkCabVehicleMotionMode::Stationary);
    Motion.Update(2.0f, 0.08f);
    TestEqual(TEXT("stable speed enters moving"), Motion.GetMode(), EPinkCabVehicleMotionMode::Moving);

    Motion.Update(0.8f, 1.0f);
    TestEqual(TEXT("hysteresis keeps moving in middle band"), Motion.GetMode(), EPinkCabVehicleMotionMode::Moving);
    Motion.Update(0.2f, 0.05f);
    TestEqual(TEXT("stationary threshold is debounced"), Motion.GetMode(), EPinkCabVehicleMotionMode::Moving);
    Motion.Update(0.2f, 0.08f);
    TestEqual(TEXT("stable low speed enters stationary"), Motion.GetMode(), EPinkCabVehicleMotionMode::Stationary);
    TestEqual(TEXT("two real mode transitions recorded"), Motion.GetTransitionSerial(), 2u);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleMotionJitterTest,
    "PinkCab.Vehicle.ControlRuntime.Motion.NoJitterChatter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleMotionJitterTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleMotionClassifier Motion;
    for (int32 Index = 0; Index < 100; ++Index)
    {
        Motion.Update((Index % 2 == 0) ? 0.9f : 1.1f, 1.0f / 60.0f);
    }
    TestEqual(TEXT("near-zero/middle-band jitter never creates a launch-driving transition"),
        Motion.GetTransitionSerial(), 0u);
    TestEqual(TEXT("jitter remains stationary"), Motion.GetMode(), EPinkCabVehicleMotionMode::Stationary);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabLaunchEdgeTest,
    "PinkCab.Vehicle.ControlRuntime.Launch.OneResetPerAttempt",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabLaunchEdgeTest::RunTest(const FString& Parameters)
{
    FPinkCabLaunchController Launch;

    TestTrue(TEXT("first E press begins first launch"), Launch.BeginLaunchAttempt());
    TestEqual(TEXT("launch serial increments once"), Launch.GetLaunchSerial(), 1u);
    TestTrue(TEXT("fresh launch requires throttle wheel dose"), Launch.RequiresThrottleDose());
    TestEqual(TEXT("fresh launch target is zero"), Launch.GetThrottleTarget(), 0.0f);

    TestTrue(TEXT("E+wheel dose is accepted"), Launch.ApplyThrottleDoseSteps(3));
    TestFalse(TEXT("dose clears required flag"), Launch.RequiresThrottleDose());
    TestTrue(TEXT("dose creates positive target"), Launch.GetThrottleTarget() > 0.0f);

    TestFalse(TEXT("held/repeated E cannot begin same launch again"), Launch.BeginLaunchAttempt());
    TestEqual(TEXT("serial remains one during same launch"), Launch.GetLaunchSerial(), 1u);

    Launch.NotifyMotionMode(EPinkCabVehicleMotionMode::Moving);
    TestFalse(TEXT("moving ends launch attempt"), Launch.IsLaunchActive());
    Launch.NotifyMotionMode(EPinkCabVehicleMotionMode::Stationary);
    TestTrue(TEXT("new E press after return to standstill begins new launch"), Launch.BeginLaunchAttempt());
    TestEqual(TEXT("new standstill departure gets new serial"), Launch.GetLaunchSerial(), 2u);
    TestTrue(TEXT("new launch requires fresh throttle dose again"), Launch.RequiresThrottleDose());
    TestEqual(TEXT("new launch resets target exactly once"), Launch.GetThrottleTarget(), 0.0f);

    const uint32 SerialBeforeJitter = Launch.GetLaunchSerial();
    for (int32 Index = 0; Index < 30; ++Index)
    {
        Launch.NotifyMotionMode(EPinkCabVehicleMotionMode::Stationary);
    }
    TestEqual(TEXT("stationary frames do not retrigger launch"), Launch.GetLaunchSerial(), SerialBeforeJitter);
    return true;
}

#endif
