#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"
#include "Vehicle/PinkCabLaunchController.h"
#include "Vehicle/PinkCabSteeringController.h"
#include "Vehicle/PinkCabPedalDosingController.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabHandbrakeActuator.h"
#include "Vehicle/PinkCabGearboxController.h"

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
    TestTrue(TEXT("center response remains precise instead of feeling like a dead zone"),
        FMath::Abs(Steering.GetTarget()) > 0.20f && FMath::Abs(Steering.GetTarget()) <= 0.25f);

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
    FPinkCabSteeringHorizontalMouseOnlyTest,
    "PinkCab.Vehicle.ControlRuntime.Steering.HorizontalMouseOnly",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSteeringHorizontalMouseOnlyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("vertical-only frame rejects stale raw MouseX"),
        FPinkCabSteeringController::ResolveHorizontalMouseDelta(0.0f, 18.0f), 0.0f);
    TestEqual(TEXT("tiny horizontal noise is treated as no steering motion"),
        FPinkCabSteeringController::ResolveHorizontalMouseDelta(0.00001f, -22.0f), 0.0f);
    TestEqual(TEXT("real horizontal frame keeps raw steering magnitude"),
        FPinkCabSteeringController::ResolveHorizontalMouseDelta(0.35f, 18.0f), 18.0f);
    TestEqual(TEXT("leftward horizontal frame keeps raw sign"),
        FPinkCabSteeringController::ResolveHorizontalMouseDelta(-0.35f, -18.0f), -18.0f);
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

    FPinkCabSteeringController SmallCorrection(Config);
    FPinkCabSteeringController LargeCorrection(Config);
    SmallCorrection.Step(50.0f, false, 80.0f, EPinkCabVehicleMotionMode::Moving, 0.02f);
    LargeCorrection.Step(100.0f, false, 80.0f, EPinkCabVehicleMotionMode::Moving, 0.02f);
    TestTrue(TEXT("smoothing preserves fine correction magnitude instead of fixed-step snapping"),
        FMath::Abs(SmallCorrection.GetSteering()) < FMath::Abs(LargeCorrection.GetSteering()));
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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPedalWheelPriorityTest,
    "PinkCab.Vehicle.ControlRuntime.Pedals.WheelPriority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPedalWheelPriorityTest::RunTest(const FString& Parameters)
{
    FPinkCabPedalDosingController Pedals;
    FPinkCabLaunchController Launch;
    FPinkCabCockpitState Cockpit;
    Launch.BeginLaunchAttempt();

    TestEqual(TEXT("E beats W and Q for wheel ownership"),
        Pedals.ApplyWheelSteps(true, true, true, 2, Launch, Cockpit),
        EPinkCabPedalWheelRecipient::Throttle);
    TestTrue(TEXT("throttle target changed"), Launch.GetThrottleTarget() > 0.0f);

    const float BrakeBefore = Pedals.GetBrakeTarget();
    TestEqual(TEXT("W beats Q when E absent"),
        Pedals.ApplyWheelSteps(true, true, false, -2, Launch, Cockpit),
        EPinkCabPedalWheelRecipient::Brake);
    TestTrue(TEXT("brake target changed only on W route"), Pedals.GetBrakeTarget() < BrakeBefore);

    const float ReleaseBefore = Cockpit.GetClutchReleaseSeconds();
    TestEqual(TEXT("Q owns wheel when alone"),
        Pedals.ApplyWheelSteps(true, false, false, 1, Launch, Cockpit),
        EPinkCabPedalWheelRecipient::ClutchRelease);
    TestTrue(TEXT("Q wheel changes release setting"), Cockpit.GetClutchReleaseSeconds() > ReleaseBefore);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPedalTargetsTest,
    "PinkCab.Vehicle.ControlRuntime.Pedals.TargetsAndCoexistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPedalTargetsTest::RunTest(const FString& Parameters)
{
    FPinkCabPedalDosingController Pedals;
    TestTrue(TEXT("default brake target is usable"), Pedals.GetBrakeTarget() > 0.0f);
    TestTrue(TEXT("default brake target remains analog"), Pedals.GetBrakeTarget() < 1.0f);

    const FPinkCabPedalTargets Both = Pedals.ResolveTargets(true, true, 0.6f);
    TestEqual(TEXT("W and E coexist: brake retained"), Both.Brake, Pedals.GetBrakeTarget());
    TestEqual(TEXT("W and E coexist: throttle retained"), Both.Throttle, 0.6f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabHandbrakeStationaryLatchTest,
    "PinkCab.Vehicle.ControlRuntime.Handbrake.StationaryLatch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabHandbrakeStationaryLatchTest::RunTest(const FString& Parameters)
{
    FPinkCabHandbrakeActuatorConfig Config;
    Config.MouseCountsForFullPull = 100.0f;
    FPinkCabHandbrakeActuator Handbrake(Config);

    Handbrake.Step(EPinkCabVehicleMotionMode::Stationary, true, -100.0f, 0.1f);
    Handbrake.Step(EPinkCabVehicleMotionMode::Stationary, false, 0.0f, 0.1f);
    TestEqual(TEXT("parking lever can be fully released"), Handbrake.GetLeverPosition(), 0.0f);

    Handbrake.Step(EPinkCabVehicleMotionMode::Stationary, true, 50.0f, 0.1f);
    Handbrake.Step(EPinkCabVehicleMotionMode::Stationary, false, 0.0f, 0.5f);
    TestTrue(TEXT("stationary RMB release latches lever"), Handbrake.IsParkingLatched());
    TestTrue(TEXT("latched parking amount is preserved"), FMath::IsNearlyEqual(Handbrake.GetLeverPosition(), 0.5f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabHandbrakeMovingReturnTest,
    "PinkCab.Vehicle.ControlRuntime.Handbrake.MovingHydraulicReturn",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabHandbrakeMovingReturnTest::RunTest(const FString& Parameters)
{
    FPinkCabHandbrakeActuatorConfig Config;
    Config.MouseCountsForFullPull = 100.0f;
    FPinkCabHandbrakeActuator Handbrake(Config);
    Handbrake.Reset(0.0f, false);

    Handbrake.Step(EPinkCabVehicleMotionMode::Moving, true, 60.0f, 0.05f);
    const float Pulled = Handbrake.GetLeverPosition();
    TestTrue(TEXT("moving pull raises hydraulic handbrake"), Pulled > 0.0f);
    TestFalse(TEXT("moving handbrake never latches"), Handbrake.IsParkingLatched());

    Handbrake.Step(EPinkCabVehicleMotionMode::Moving, false, 0.0f, 0.05f);
    TestTrue(TEXT("RMB release begins automatic return"), Handbrake.GetLeverPosition() < Pulled);
    for (int32 Index = 0; Index < 30; ++Index)
    {
        Handbrake.Step(EPinkCabVehicleMotionMode::Moving, false, 0.0f, 0.05f);
    }
    TestEqual(TEXT("moving auto-return reaches zero"), Handbrake.GetLeverPosition(), 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabHandbrakeCurveTest,
    "PinkCab.Vehicle.ControlRuntime.Handbrake.NonlinearTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabHandbrakeCurveTest::RunTest(const FString& Parameters)
{
    FPinkCabHandbrakeActuatorConfig Config;
    Config.MouseCountsForFullPull = 100.0f;

    FPinkCabHandbrakeActuator Quarter(Config);
    Quarter.Reset(0.0f, false);
    const float Q = Quarter.Step(EPinkCabVehicleMotionMode::Moving, true, 25.0f, 0.01f);

    FPinkCabHandbrakeActuator Half(Config);
    Half.Reset(0.0f, false);
    const float H = Half.Step(EPinkCabVehicleMotionMode::Moving, true, 50.0f, 0.01f);

    FPinkCabHandbrakeActuator Full(Config);
    Full.Reset(0.0f, false);
    const float F = Full.Step(EPinkCabVehicleMotionMode::Moving, true, 100.0f, 0.01f);

    TestTrue(TEXT("25/50/100 commands are distinct"), Q < H && H < F);
    TestTrue(TEXT("fine first half is softer than linear"), H < 0.5f);
    TestEqual(TEXT("full pull reaches full torque command"), F, 1.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabHGateTopologyTest,
    "PinkCab.Vehicle.ControlRuntime.Gearbox.HGateTopology",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabHGateTopologyTest::RunTest(const FString& Parameters)
{
    FPinkCabGearboxController Gearbox;
    TestTrue(TEXT("neutral to first through top-left slot"), Gearbox.MoveGate(-1.0f, 1.0f));
    TestEqual(TEXT("top-left requests first"), Gearbox.GetRequestedGear(), 1);
    TestFalse(TEXT("top-row lateral wall blocks first-to-third shortcut"),
        Gearbox.MoveGate(0.0f, 1.0f));
    TestEqual(TEXT("lateral wall preserves first"), Gearbox.GetRequestedGear(), 1);

    TestFalse(TEXT("direct top-to-bottom teleport is rejected"), Gearbox.MoveGate(-1.0f, -1.0f));
    TestEqual(TEXT("rejected diagonal/vertical wall crossing preserves request"), Gearbox.GetRequestedGear(), 1);

    TestTrue(TEXT("neutral corridor is reachable"), Gearbox.MoveGate(-1.0f, 0.0f));
    TestEqual(TEXT("corridor requests neutral"), Gearbox.GetRequestedGear(), 0);
    TestTrue(TEXT("after neutral traversal bottom-left is second"), Gearbox.MoveGate(-1.0f, -1.0f));
    TestEqual(TEXT("bottom-left requests second"), Gearbox.GetRequestedGear(), 2);

    TestTrue(TEXT("return to neutral"), Gearbox.MoveGate(0.0f, 0.0f));
    TestTrue(TEXT("top-center is third"), Gearbox.MoveGate(0.0f, 1.0f));
    TestEqual(TEXT("third requested"), Gearbox.GetRequestedGear(), 3);
    TestTrue(TEXT("neutral before right column"), Gearbox.MoveGate(0.0f, 0.0f));
    TestTrue(TEXT("top-right is fifth"), Gearbox.MoveGate(1.0f, 1.0f));
    TestEqual(TEXT("fifth requested"), Gearbox.GetRequestedGear(), 5);
    TestTrue(TEXT("neutral before reverse"), Gearbox.MoveGate(1.0f, 0.0f));
    TestTrue(TEXT("bottom-right is reverse"), Gearbox.MoveGate(1.0f, -1.0f));
    TestEqual(TEXT("reverse requested"), Gearbox.GetRequestedGear(), -1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGearEngagementValidatorTest,
    "PinkCab.Vehicle.ControlRuntime.Gearbox.EngagementValidator",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGearEngagementValidatorTest::RunTest(const FString& Parameters)
{
    FPinkCabGearboxController Gearbox;
    FPinkCabGearEngagementContext Context;
    Context.ClutchPedal = 1.0f;
    Context.EngineRpm = 900.0f;
    Context.SpeedKmh = 0.0f;
    TestTrue(TEXT("request first accepted with clutch disengaged"), Gearbox.RequestGear(1, Context));
    TestEqual(TEXT("requested gear records first"), Gearbox.GetRequestedGear(), 1);
    TestEqual(TEXT("engaged gear becomes first"), Gearbox.GetEngagedGear(), 1);

    Context.ClutchPedal = 0.0f;
    Context.EngineRpm = 4000.0f;
    Context.SpeedKmh = 35.0f;
    Context.Throttle = 0.7f;
    TestFalse(TEXT("loaded mismatched clutchless second is refused"), Gearbox.RequestGear(2, Context));
    TestEqual(TEXT("refusal keeps old engaged gear"), Gearbox.GetEngagedGear(), 1);
    TestEqual(TEXT("request remains observable after grind"), Gearbox.GetRequestedGear(), 2);
    TestEqual(TEXT("mismatch classifies as grind"), Gearbox.GetLastResult(), EPinkCabGearEngagementResult::GrindRefused);

    Context.ClutchPedal = 1.0f;
    Gearbox.EvaluateCurrentEngagement(Context);
    TestEqual(TEXT("previously refused request engages after clutch is pressed"), Gearbox.GetEngagedGear(), 2);
    TestEqual(TEXT("retry is classified as clutch-disengaged acceptance"),
        Gearbox.GetLastResult(), EPinkCabGearEngagementResult::ClutchDisengagedAccepted);

    FPinkCabGearboxController Matched;
    Context.ClutchPedal = 0.0f;
    Context.SpeedKmh = 20.0f;
    Context.EngineRpm = Matched.ExpectedEngineRpmForGear(2, Context.SpeedKmh);
    Context.Throttle = 0.05f;
    TestTrue(TEXT("matched low-load clutchless engagement is possible"), Matched.RequestGear(2, Context));
    TestEqual(TEXT("matched request actually engages"), Matched.GetEngagedGear(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabContinuousClutchTest,
    "PinkCab.Vehicle.ControlRuntime.Gearbox.ContinuousClutch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabContinuousClutchTest::RunTest(const FString& Parameters)
{
    FPinkCabGearboxController Gearbox;
    const float C0 = Gearbox.ComputeClutchCoupling(1.0f);
    const float C25 = Gearbox.ComputeClutchCoupling(0.75f);
    const float C50 = Gearbox.ComputeClutchCoupling(0.50f);
    const float C75 = Gearbox.ComputeClutchCoupling(0.25f);
    const float C100 = Gearbox.ComputeClutchCoupling(0.0f);
    TestEqual(TEXT("fully pressed clutch transfers zero torque"), C0, 0.0f);
    TestTrue(TEXT("clutch transfer is continuous and monotonic"), C0 < C25 && C25 < C50 && C50 < C75 && C75 < C100);
    TestTrue(TEXT("half clutch remains partial rather than binary"), C50 > 0.0f && C50 < 1.0f);
    TestEqual(TEXT("released clutch reaches full coupling"), C100, 1.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabDangerousDownshiftTest,
    "PinkCab.Vehicle.ControlRuntime.Gearbox.DangerousDownshift",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabDangerousDownshiftTest::RunTest(const FString& Parameters)
{
    FPinkCabGearboxController Gearbox;
    FPinkCabGearEngagementContext Context;
    Context.ClutchPedal = 1.0f;
    Context.SpeedKmh = 100.0f;
    Context.EngineRpm = 2500.0f;
    TestTrue(TEXT("fifth may be selected with clutch disengaged"), Gearbox.RequestGear(5, Context));
    TestTrue(TEXT("first may be physically selected while clutch remains disengaged"), Gearbox.RequestGear(1, Context));

    Context.ClutchPedal = 0.0f;
    Context.EngineRpm = 2500.0f;
    Context.Throttle = 0.0f;
    Gearbox.EvaluateCurrentEngagement(Context);
    TestEqual(TEXT("releasing clutch into impossible downshift reports dangerous overrev"),
        Gearbox.GetLastResult(), EPinkCabGearEngagementResult::DangerousOverrev);
    TestTrue(TEXT("dangerous classification exposes overspeed rpm"),
        Gearbox.GetExpectedCoupledRpm() > Gearbox.GetMaxSafeEngineRpm());
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGearboxPhysicalMouseAndCancelTest,
    "PinkCab.Vehicle.ControlRuntime.Gearbox.PhysicalMouseAndCancel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGearboxPhysicalMouseAndCancelTest::RunTest(const FString& Parameters)
{
    FPinkCabGearboxController Gearbox;
    TestFalse(TEXT("moving lever left inside neutral remains neutral"),
        Gearbox.ApplyLeverMouseDelta(-160.0f, 0.0f));
    TestTrue(TEXT("UE raw MouseY positive/up moves lever forward into first"),
        Gearbox.ApplyLeverMouseDelta(0.0f, 140.0f));
    TestEqual(TEXT("mouse H-gate reaches first"), Gearbox.GetRequestedGear(), 1);

    Gearbox.ApplyLeverMouseDelta(160.0f, 0.0f);
    TestEqual(TEXT("horizontal mouse cannot cut across top H wall"),
        Gearbox.GetRequestedGear(), 1);

    FPinkCabGearEngagementContext Context;
    Context.ClutchPedal = 1.0f;
    TestTrue(TEXT("first engages with clutch"), Gearbox.RequestGear(1, Context));
    Context.ClutchPedal = 0.0f;
    Context.EngineRpm = 4000.0f;
    Context.SpeedKmh = 35.0f;
    Context.Throttle = 0.8f;
    TestFalse(TEXT("mismatched second creates pending request"), Gearbox.RequestGear(2, Context));
    TestEqual(TEXT("second remains requested before cleanup"), Gearbox.GetRequestedGear(), 2);
    Gearbox.CancelPendingRequest();
    TestEqual(TEXT("focus cleanup returns lever request to engaged first"),
        Gearbox.GetRequestedGear(), 1);
    TestEqual(TEXT("focus cleanup never changes actual engaged gear"),
        Gearbox.GetEngagedGear(), 1);
    return true;
}


#endif
