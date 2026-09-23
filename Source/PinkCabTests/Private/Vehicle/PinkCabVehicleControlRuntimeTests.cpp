#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleControlRuntime.h"
#include "Vehicle/PinkCabSteeringController.h"

namespace
{
FPinkCabVehicleTelemetry Telemetry(float SpeedKmh = 0.0f, float EngineRpm = 750.0f)
{
    FPinkCabVehicleTelemetry Result;
    Result.SpeedKmh = SpeedKmh;
    Result.EngineRpm = EngineRpm;
    return Result;
}

FPinkCabVehicleControlTickInput Digital(
    bool bClutch,
    bool bBrake,
    bool bThrottle,
    int32 WheelSteps = 0,
    float SteeringMouseX = 0.0f,
    float DeltaSeconds = 1.0f / 60.0f)
{
    FPinkCabVehicleControlTickInput Input;
    Input.Frame = FPinkCabVehicleInputFrame::FromDigital(false, bClutch, bBrake, bThrottle);
    Input.WheelSteps = WheelSteps;
    Input.SteeringMouseX = SteeringMouseX;
    Input.DeltaSeconds = DeltaSeconds;
    return Input;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeLaunchTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.LaunchDoseAndReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeLaunchTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;

    Runtime.Update(Digital(true, false, true), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("first E press starts exactly one launch"), Runtime.GetLaunchSerial(), 1u);
    TestTrue(TEXT("fresh launch requires explicit E+wheel throttle dose"), Runtime.RequiresThrottleDose());
    TestEqual(TEXT("fresh E press alone keeps throttle target at zero"),
        Runtime.GetThrottleTarget(), 0.0f);
    TestTrue(TEXT("Q remains active while E owns throttle dosing"),
        Runtime.GetControlState().Clutch > 0.0f);
    TestEqual(TEXT("E without wheel does not invent throttle"),
        Runtime.GetControlState().Throttle, 0.0f);

    Runtime.Update(Digital(true, false, true, 1), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("one E+wheel step sets exactly five percent throttle"),
        Runtime.GetThrottleTarget(), 0.05f);
    TestFalse(TEXT("positive throttle dose satisfies new-launch requirement"), Runtime.RequiresThrottleDose());
    TestTrue(TEXT("Q and dosed E coexist in the same smoothed control frame"),
        Runtime.GetControlState().Clutch > 0.0f && Runtime.GetControlState().Throttle > 0.0f);

    Runtime.Update(Digital(true, false, true), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("held E never resets launch again"), Runtime.GetLaunchSerial(), 1u);
    TestEqual(TEXT("held E preserves the explicit wheel-set dose"), Runtime.GetThrottleTarget(), 0.05f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeRepeatedLaunchTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.RepeatedLaunchAfterMoveStop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeRepeatedLaunchTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;

    Runtime.Update(Digital(true, false, true, 1, 0.0f, 0.20f), Telemetry(0.0f), Cockpit, Health);
    TestEqual(TEXT("first launch serial"), Runtime.GetLaunchSerial(), 1u);
    TestEqual(TEXT("first launch one wheel step is five percent"),
        Runtime.GetThrottleTarget(), 0.05f);

    Runtime.Update(Digital(true, false, false, 0, 0.0f, 0.20f), Telemetry(5.0f), Cockpit, Health);
    TestEqual(TEXT("moving telemetry exits stationary launch mode"),
        Runtime.GetMotionMode(), EPinkCabVehicleMotionMode::Moving);

    Runtime.Update(Digital(true, false, false, 0, 0.0f, 0.20f), Telemetry(0.0f), Cockpit, Health);
    TestEqual(TEXT("stopped telemetry returns to stationary"),
        Runtime.GetMotionMode(), EPinkCabVehicleMotionMode::Stationary);

    Runtime.Update(Digital(true, false, true, 0, 0.0f, 0.20f), Telemetry(0.0f), Cockpit, Health);
    TestEqual(TEXT("fresh E after stop starts exactly second launch"), Runtime.GetLaunchSerial(), 2u);
    TestTrue(TEXT("second launch requires fresh wheel dose"), Runtime.RequiresThrottleDose());
    TestEqual(TEXT("second launch resets target to zero once"),
        Runtime.GetThrottleTarget(), 0.0f);

    Runtime.Update(Digital(true, false, true, 1, 0.0f, 0.20f), Telemetry(0.0f), Cockpit, Health);
    TestEqual(TEXT("second launch one wheel step is five percent"),
        Runtime.GetThrottleTarget(), 0.05f);
    TestFalse(TEXT("second launch dose satisfies requirement"), Runtime.RequiresThrottleDose());

    Runtime.Update(Digital(true, false, true, 0, 0.0f, 0.20f), Telemetry(0.0f), Cockpit, Health);
    TestEqual(TEXT("held E does not reset second launch target"),
        Runtime.GetThrottleTarget(), 0.05f);
    TestEqual(TEXT("held E does not create third launch"), Runtime.GetLaunchSerial(), 2u);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeWheelPriorityTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.WheelPriority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeWheelPriorityTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;

    Runtime.Update(Digital(true, true, true, 1), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("E owns wheel before W/Q"), Runtime.GetLastWheelRecipient(), EPinkCabPedalWheelRecipient::Throttle);
    TestEqual(TEXT("E wheel-up owns the new-launch dose and sets five percent"),
        Runtime.GetThrottleTarget(), 0.05f);

    const float BrakeBefore = Runtime.GetBrakeTarget();
    Runtime.Update(Digital(true, true, false, -1), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("W owns wheel when E absent"), Runtime.GetLastWheelRecipient(), EPinkCabPedalWheelRecipient::Brake);
    TestTrue(TEXT("W step changes brake target"), Runtime.GetBrakeTarget() < BrakeBefore);

    const float ReleaseBefore = Cockpit.GetClutchReleaseSeconds();
    Runtime.Update(Digital(true, false, false, 1), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("Q owns wheel when E/W absent"), Runtime.GetLastWheelRecipient(), EPinkCabPedalWheelRecipient::ClutchRelease);
    TestTrue(TEXT("Q wheel-up makes clutch release faster"), Cockpit.GetClutchReleaseSeconds() < ReleaseBefore);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeSteeringTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.SteeringSign",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeSteeringTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;
    Runtime.Update(Digital(false, false, false, 0, 400.0f, 0.2f), Telemetry(40.0f), Cockpit, Health);
    TestTrue(TEXT("positive driver-right mouse produces positive steering"), Runtime.GetSteeringCommand() > 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeGearboxTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.GearboxRequestedEngaged",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeGearboxTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;

    Runtime.ApplyPhysicalControl(FName(TEXT("Gearbox")), true, -160.0f, 0.0f, 0.05f, Cockpit);
    Runtime.ApplyPhysicalControl(FName(TEXT("Gearbox")), true, 0.0f, 140.0f, 0.05f, Cockpit);
    TestEqual(TEXT("top-left H slot requests first"), Runtime.GetRequestedGear(), 1);
    TestEqual(TEXT("physical request does not magically engage gear"), Runtime.GetEngagedGear(), 0);

    Runtime.Update(Digital(true, false, false, 0, 0.0f, 1.0f), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("pressed clutch accepts requested first"), Runtime.GetEngagedGear(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeHandbrakeTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.AnalogHandbrake",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeHandbrakeTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    Runtime.ResetHandbrake(0.0f, false, Cockpit);
    Runtime.ApplyPhysicalControl(FName(TEXT("Handbrake")), true, 0.0f, -110.0f, 0.05f, Cockpit);
    TestTrue(TEXT("half physical pull has nonzero brake command"), Runtime.GetHandbrakeCommand() > 0.0f);
    TestTrue(TEXT("half physical pull remains analog below full"), Runtime.GetHandbrakeCommand() < 1.0f);
    TestTrue(TEXT("cockpit gets analog lever position"), Cockpit.GetHandbrakeAmount() > 0.0f && Cockpit.GetHandbrakeAmount() < 1.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeStallTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.LowRpmStall",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeStallTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;
    Cockpit.StartEngine();
    Runtime.ForceGearState(1, 1, Cockpit);

    Runtime.Update(Digital(false, false, false, 0, 0.0f, 1.0f), Telemetry(0.0f, 800.0f), Cockpit, Health);
    TestEqual(TEXT("fully coupled first at sub-stall rpm stalls engine"), Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Stalled);
    TestEqual(TEXT("displayed rpm drops to zero on stall"), Runtime.GetDisplayedEngineRpm(), 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeTransientResetTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.TransientReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeTransientResetTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;
    Runtime.ForceGearState(2, 1, Cockpit);
    TestEqual(TEXT("fixture starts with pending second over engaged first"), Runtime.GetRequestedGear(), 2);
    Runtime.ResetTransient(Cockpit);
    TestEqual(TEXT("transient reset returns request to engaged gear"), Runtime.GetRequestedGear(), Runtime.GetEngagedGear());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleControlRuntimeSteeringHoldTest,
    "PinkCab.Vehicle.ControlRuntime.Runtime.SteeringHoldDuringManipulation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleControlRuntimeSteeringHoldTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;

    FPinkCabVehicleInputFrame Frame =
        FPinkCabVehicleInputFrame::FromDigital(false, false, false, false);
    Runtime.ResolveControlFrame(Frame, 900.0f, 0.05f, Cockpit, Health);
    const float BeforeManipulation = Runtime.GetSteeringCommand();
    TestTrue(TEXT("precondition produces nonzero steering"), FMath::Abs(BeforeManipulation) > KINDA_SMALL_NUMBER);

    Frame.bSteeringHeld = true;
    Runtime.ResolveControlFrame(Frame, 0.0f, 0.50f, Cockpit, Health);
    TestTrue(TEXT("lever manipulation preserves exact current steering angle"),
        FMath::IsNearlyEqual(Runtime.GetSteeringCommand(), BeforeManipulation, 1.0e-6f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabManualSteeringWeightTest,
    "PinkCab.Vehicle.Steering.ManualSteeringWeight",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabManualSteeringWeightTest::RunTest(const FString& Parameters)
{
    FPinkCabSteeringController Steering;

    const float MouseDelta = 280.0f;
    const float DeltaSeconds = 0.20f;

    Steering.Reset();
    const float Stationary = FMath::Abs(Steering.Step(
        MouseDelta, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, DeltaSeconds));

    Steering.Reset();
    const float Rolling = FMath::Abs(Steering.Step(
        MouseDelta, false, 10.0f, EPinkCabVehicleMotionMode::Moving, DeltaSeconds));

    Steering.Reset();
    const float Highway = FMath::Abs(Steering.Step(
        MouseDelta, false, 120.0f, EPinkCabVehicleMotionMode::Moving, DeltaSeconds));

    TestTrue(TEXT("manual steering is heavier at a standstill than once rolling"),
        Stationary < Rolling);
    TestTrue(TEXT("high-speed steering is calmer than low-speed rolling steering"),
        Highway < Rolling);
    TestTrue(TEXT("stationary response rate is lower than rolling response"),
        Steering.GetResponseRate(0.0f, EPinkCabVehicleMotionMode::Stationary)
            < Steering.GetResponseRate(10.0f, EPinkCabVehicleMotionMode::Moving));
    TestTrue(TEXT("high-speed response does not accelerate with speed"),
        Steering.GetResponseRate(120.0f, EPinkCabVehicleMotionMode::Moving)
            < Steering.GetResponseRate(10.0f, EPinkCabVehicleMotionMode::Moving));
    return true;
}

#endif
