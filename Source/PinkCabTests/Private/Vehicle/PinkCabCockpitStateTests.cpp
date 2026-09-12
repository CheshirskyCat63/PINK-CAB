#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabCockpitState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitDefaultsTest,
    "PinkCab.Vehicle.Cockpit.Defaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitDefaultsTest::RunTest(const FString& Parameters)
{
    const FPinkCabCockpitState State;
    TestEqual(TEXT("engine starts off"), State.GetIgnitionState(), EPinkCabIgnitionState::Off);
    TestEqual(TEXT("gear starts neutral"), State.GetSelectedGear(), 0);
    TestTrue(TEXT("parking handbrake starts engaged"), State.IsHandbrakeEngaged());
    TestFalse(TEXT("passenger door starts closed"), State.IsPassengerDoorOpen());
    TestEqual(TEXT("meter starts off"), State.GetMeterState(), EPinkCabMeterState::Off);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitIgnitionTest,
    "PinkCab.Vehicle.Cockpit.Ignition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabCockpitIgnitionTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitState State;
    TestTrue(TEXT("engine starts"), State.StartEngine());
    TestEqual(TEXT("running state"), State.GetIgnitionState(), EPinkCabIgnitionState::Running);
    State.StallEngine();
    TestEqual(TEXT("stall state"), State.GetIgnitionState(), EPinkCabIgnitionState::Stalled);
    TestTrue(TEXT("stalled engine restarts"), State.StartEngine());
    State.StopEngine();
    TestEqual(TEXT("engine stops"), State.GetIgnitionState(), EPinkCabIgnitionState::Off);
    State.StopEngine();
    TestEqual(TEXT("stop is idempotent"), State.GetIgnitionState(), EPinkCabIgnitionState::Off);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitGearAndHandbrakeTest,
    "PinkCab.Vehicle.Cockpit.GearAndHandbrake",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitGearAndHandbrakeTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitState State;
    TestEqual(TEXT("shift to first"), State.ShiftBy(1), 1);
    TestEqual(TEXT("shift to fifth clamps"), State.ShiftBy(99), 5);
    TestEqual(TEXT("shift to reverse clamps"), State.ShiftBy(-99), -1);
    State.SetHandbrakeEngaged(false);
    TestFalse(TEXT("handbrake releases"), State.IsHandbrakeEngaged());
    State.SetHandbrakeEngaged(true);
    TestTrue(TEXT("handbrake engages"), State.IsHandbrakeEngaged());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitDoorAndMeterTest,
    "PinkCab.Vehicle.Cockpit.DoorAndMeter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitDoorAndMeterTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitState State;
    State.SetPassengerDoorOpen(true);
    TestTrue(TEXT("passenger door opens"), State.IsPassengerDoorOpen());
    State.SetPassengerDoorOpen(false);
    TestFalse(TEXT("passenger door closes"), State.IsPassengerDoorOpen());

    TestTrue(TEXT("meter starts"), State.StartMeter());
    TestEqual(TEXT("meter running"), State.GetMeterState(), EPinkCabMeterState::Running);
    TestTrue(TEXT("meter stops"), State.StopMeter());
    TestEqual(TEXT("meter stopped"), State.GetMeterState(), EPinkCabMeterState::Stopped);
    State.ResetMeter();
    State.ResetMeter();
    TestEqual(TEXT("meter reset is idempotent"), State.GetMeterState(), EPinkCabMeterState::Off);
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

#include "Vehicle/PinkCabCockpitInteractionRouter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitInteractionRouterTest,
    "PinkCab.Vehicle.Cockpit.InteractionRouter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabCockpitInteractionRouterTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitState State;

    TestTrue(TEXT("gearbox wheel event is consumed"),
        FPinkCabCockpitInteractionRouter::Apply(
            {FName(TEXT("Gearbox")), EPinkCabInteractionGesture::WheelIncrement, 1}, State));
    TestEqual(TEXT("gearbox event selects first"), State.GetSelectedGear(), 1);

    TestTrue(TEXT("handbrake press is consumed"),
        FPinkCabCockpitInteractionRouter::Apply(
            {FName(TEXT("Handbrake")), EPinkCabInteractionGesture::PressHold, 1}, State));
    TestFalse(TEXT("handbrake press toggles release"), State.IsHandbrakeEngaged());
    TestFalse(TEXT("handbrake release does not toggle again"),
        FPinkCabCockpitInteractionRouter::Apply(
            {FName(TEXT("Handbrake")), EPinkCabInteractionGesture::PressHold, 0}, State));

    TestTrue(TEXT("ignition press starts engine"),
        FPinkCabCockpitInteractionRouter::Apply(
            {FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}, State));
    TestEqual(TEXT("ignition event reaches running"), State.GetIgnitionState(), EPinkCabIgnitionState::Running);

    TestTrue(TEXT("passenger door press toggles door"),
        FPinkCabCockpitInteractionRouter::Apply(
            {FName(TEXT("PassengerDoor")), EPinkCabInteractionGesture::PressHold, 1}, State));
    TestTrue(TEXT("passenger door is open"), State.IsPassengerDoorOpen());

    TestTrue(TEXT("meter press starts fare"),
        FPinkCabCockpitInteractionRouter::Apply(
            {FName(TEXT("Meter")), EPinkCabInteractionGesture::PressHold, 1}, State));
    TestEqual(TEXT("meter runs after first press"), State.GetMeterState(), EPinkCabMeterState::Running);
    FPinkCabCockpitInteractionRouter::Apply(
        {FName(TEXT("Meter")), EPinkCabInteractionGesture::PressHold, 1}, State);
    TestEqual(TEXT("meter stops after second press"), State.GetMeterState(), EPinkCabMeterState::Stopped);
    FPinkCabCockpitInteractionRouter::Apply(
        {FName(TEXT("Meter")), EPinkCabInteractionGesture::PressHold, 1}, State);
    TestEqual(TEXT("meter resets after third press"), State.GetMeterState(), EPinkCabMeterState::Off);

    return true;
}

#endif
