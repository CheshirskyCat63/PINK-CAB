#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleControlRuntime.h"

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
    TestFalse(TEXT("fresh launch never requires wheel permission"), Runtime.RequiresThrottleDose());
    TestEqual(TEXT("fresh E press immediately gives the authored 45 percent throttle target"),
        Runtime.GetThrottleTarget(), 0.45f);
    TestTrue(TEXT("Q and E coexist in the same smoothed control frame"),
        Runtime.GetControlState().Clutch > 0.0f && Runtime.GetControlState().Throttle > 0.0f);

    Runtime.Update(Digital(true, false, true, 1), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("one E+wheel step fine-adjusts throttle by five percent"),
        Runtime.GetThrottleTarget(), 0.50f);
    TestFalse(TEXT("wheel adjustment is never mandatory launch permission"), Runtime.RequiresThrottleDose());

    Runtime.Update(Digital(true, false, true), Telemetry(), Cockpit, Health);
    TestEqual(TEXT("held E never resets launch again"), Runtime.GetLaunchSerial(), 1u);
    TestEqual(TEXT("held E preserves adjusted throttle target"), Runtime.GetThrottleTarget(), 0.50f);
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
    TestEqual(TEXT("E dose increments 5%"), Runtime.GetThrottleTarget(), 0.05f);

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

#endif
