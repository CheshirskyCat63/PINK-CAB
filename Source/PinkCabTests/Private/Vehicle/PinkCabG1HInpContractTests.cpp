#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Interaction/PinkCabPhysicalInputConvention.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabHGateGeometry.h"
#include "Vehicle/PinkCabLaunchController.h"
#include "Vehicle/PinkCabPedalDosingController.h"

namespace
{
int32 GearFromNeutral(const float DriverRightCounts, const float DriverForwardCounts)
{
    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 0);
    FPinkCabHGateGeometry::ApplyDriverDelta(
        State, DriverRightCounts, DriverForwardCounts);
    return State.RequestedGear;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabG1HInpPhysicalDirectionContractTest,
    "PinkCab.G1.HInp.PhysicalDirections",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabG1HInpPhysicalDirectionContractTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("positive mouse X remains driver-right steering"),
        FPinkCabPhysicalInputConvention::SteeringRight(7.0f), 7.0f);
    TestEqual(TEXT("positive device Y remains gearbox-forward"),
        FPinkCabPhysicalInputConvention::GearboxForward(9.0f), 9.0f);
    TestEqual(TEXT("negative device Y means positive handbrake pull"),
        FPinkCabPhysicalInputConvention::HandbrakePull(-11.0f), 11.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabG1HInpHGateSlotsTest,
    "PinkCab.G1.HInp.HGate.AllSlotsAndDirections",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabG1HInpHGateSlotsTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("driver left + forward selects first"), GearFromNeutral(-160.0f, 140.0f), 1);
    TestEqual(TEXT("driver left + back selects second"), GearFromNeutral(-160.0f, -140.0f), 2);
    TestEqual(TEXT("driver center + forward selects third"), GearFromNeutral(0.0f, 140.0f), 3);
    TestEqual(TEXT("driver center + back selects fourth"), GearFromNeutral(0.0f, -140.0f), 4);
    TestEqual(TEXT("driver right + forward selects fifth"), GearFromNeutral(160.0f, 140.0f), 5);
    TestEqual(TEXT("driver right + back selects reverse"), GearFromNeutral(160.0f, -140.0f), -1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabG1HInpHGateNeutralCrossTest,
    "PinkCab.G1.HInp.HGate.NeutralCrossGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabG1HInpHGateNeutralCrossTest::RunTest(const FString& Parameters)
{
    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 5);

    FPinkCabHGateGeometry::ApplyDriverDelta(State, -320.0f, 0.0f);
    TestEqual(TEXT("lever cannot change column while still in fifth row"),
        State.RequestedGear, 5);

    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -140.0f);
    TestEqual(TEXT("leaving fifth reaches neutral before bottom row"),
        State.RequestedGear, 0);

    FPinkCabHGateGeometry::ApplyDriverDelta(State, -320.0f, 0.0f);
    TestEqual(TEXT("neutral cross-gate can move to left column without selecting gear"),
        State.RequestedGear, 0);

    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -140.0f);
    TestEqual(TEXT("left column bottom selects second only after neutral"),
        State.RequestedGear, 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabG1HInpClutchReleaseWheelDirectionTest,
    "PinkCab.G1.HInp.ClutchReleaseWheelDirection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabG1HInpClutchReleaseWheelDirectionTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitState Cockpit;
    FPinkCabLaunchController Launch;
    FPinkCabPedalDosingController Dosing;

    const float DefaultSeconds = Cockpit.GetClutchReleaseSeconds();
    TestTrue(TEXT("default clutch-release time is inside authored bounds"),
        DefaultSeconds > FPinkCabCockpitState::ClutchReleaseMinSeconds
        && DefaultSeconds < FPinkCabCockpitState::ClutchReleaseMaxSeconds);

    const EPinkCabPedalWheelRecipient UpRecipient = Dosing.ApplyWheelSteps(
        true, false, false, 1, Launch, Cockpit);
    const float FasterSeconds = Cockpit.GetClutchReleaseSeconds();
    TestEqual(TEXT("Q owns wheel-up as clutch-release adjustment"),
        UpRecipient, EPinkCabPedalWheelRecipient::ClutchRelease);
    TestTrue(TEXT("Q+wheel up makes clutch release faster by reducing seconds"),
        FasterSeconds < DefaultSeconds);

    const EPinkCabPedalWheelRecipient DownRecipient = Dosing.ApplyWheelSteps(
        true, false, false, -1, Launch, Cockpit);
    const float SlowerSeconds = Cockpit.GetClutchReleaseSeconds();
    TestEqual(TEXT("Q owns wheel-down as clutch-release adjustment"),
        DownRecipient, EPinkCabPedalWheelRecipient::ClutchRelease);
    TestTrue(TEXT("Q+wheel down makes clutch release slower by increasing seconds"),
        SlowerSeconds > FasterSeconds);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabG1HInpDosingParametersTest,
    "PinkCab.G1.HInp.DosingParameters",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabG1HInpDosingParametersTest::RunTest(const FString& Parameters)
{
    const FPinkCabPedalDosingControllerConfig BrakeConfig;
    TestEqual(TEXT("brake default remains 45 percent"), BrakeConfig.BrakeDefault, 0.45f);
    TestEqual(TEXT("brake wheel step remains five percent"), BrakeConfig.BrakeStep, 0.05f);
    TestEqual(TEXT("throttle wheel step remains five percent"),
        FPinkCabLaunchController::ThrottleDoseStep, 0.05f);
    TestEqual(TEXT("clutch release lower bound remains 0.20 seconds"),
        FPinkCabCockpitState::ClutchReleaseMinSeconds, 0.20f);
    TestEqual(TEXT("clutch release upper bound remains 1.20 seconds"),
        FPinkCabCockpitState::ClutchReleaseMaxSeconds, 1.20f);
    return true;
}

#endif
