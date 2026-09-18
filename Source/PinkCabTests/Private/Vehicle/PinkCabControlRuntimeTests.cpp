#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"
#include "Vehicle/PinkCabLaunchController.h"
#include "Vehicle/PinkCabSteeringController.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSteeringTransferTest,
    "PinkCab.Vehicle.ControlRuntime.Steering.Transfer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSteeringTransferTest::RunTest(const FString& Parameters)
{
    FPinkCabSteeringControllerConfig Config;
    Config.MouseCountsForFullScale = 100.0f;
    FPinkCabSteeringController Steering(Config);

    Steering.Step(25.0f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 0.0f);
    TestEqual(TEXT("quarter mouse travel maps to quarter virtual cursor"), Steering.GetVirtualCursor(), 0.25f);
    TestTrue(TEXT("nonlinear center target stays below linear cursor"), FMath::Abs(Steering.GetTarget()) < 0.25f);

    const float PositiveTarget = Steering.GetTarget();
    Steering.Reset();
    Steering.Step(-25.0f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 0.0f);
    TestTrue(TEXT("target curve is odd symmetric"), FMath::IsNearlyEqual(Steering.GetTarget(), -PositiveTarget, 1.e-4f));

    Steering.Reset();
    Steering.Step(10000.0f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 0.0f);
    TestEqual(TEXT("virtual cursor clamps positive"), Steering.GetVirtualCursor(), 1.0f);
    Steering.Step(-20000.0f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 0.0f);
    TestEqual(TEXT("virtual cursor clamps negative"), Steering.GetVirtualCursor(), -1.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSteeringSpeedResponseTest,
    "PinkCab.Vehicle.ControlRuntime.Steering.SpeedResponse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSteeringSpeedResponseTest::RunTest(const FString& Parameters)
{
    FPinkCabSteeringControllerConfig Config;
    Config.MouseCountsForFullScale = 100.0f;

    FPinkCabSteeringController Stationary(Config);
    FPinkCabSteeringController MovingSlow(Config);
    FPinkCabSteeringController MovingFast(Config);
    Stationary.Step(100.0f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 0.10f);
    MovingSlow.Step(100.0f, false, 20.0f, EPinkCabVehicleMotionMode::Moving, 0.10f);
    MovingFast.Step(100.0f, false, 140.0f, EPinkCabVehicleMotionMode::Moving, 0.10f);

    TestTrue(TEXT("stationary wheel responds heavier/slower than moving"), FMath::Abs(Stationary.GetSteering()) < FMath::Abs(MovingSlow.GetSteering()));
    TestTrue(TEXT("high speed steering response is sharper than low speed"), FMath::Abs(MovingSlow.GetSteering()) < FMath::Abs(MovingFast.GetSteering()));
    TestTrue(TEXT("high speed still stays bounded"), FMath::Abs(MovingFast.GetSteering()) <= 1.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSteeringGazeOwnershipTest,
    "PinkCab.Vehicle.ControlRuntime.Steering.GazeOwnership",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSteeringGazeOwnershipTest::RunTest(const FString& Parameters)
{
    FPinkCabSteeringControllerConfig Config;
    Config.MouseCountsForFullScale = 100.0f;
    FPinkCabSteeringController Steering(Config);
    Steering.Step(50.0f, false, 60.0f, EPinkCabVehicleMotionMode::Moving, 0.2f);

    const float CursorBefore = Steering.GetVirtualCursor();
    const float SteeringBefore = Steering.GetSteering();
    Steering.Step(100.0f, true, 120.0f, EPinkCabVehicleMotionMode::Moving, 1.0f);
    TestEqual(TEXT("gaze owns mouse and freezes steering cursor"), Steering.GetVirtualCursor(), CursorBefore);
    TestEqual(TEXT("gaze ownership does not auto-steer"), Steering.GetSteering(), SteeringBefore);
    return true;
}


#endif
