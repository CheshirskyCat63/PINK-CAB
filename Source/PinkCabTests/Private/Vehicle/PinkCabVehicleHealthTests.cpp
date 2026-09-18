#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleHitEvent.h"
#include "Vehicle/PinkCabVehicleHealthService.h"
#include "Vehicle/PinkCabDrivetrainCondition.h"
#include "Vehicle/PinkCabVehicleHealthBinding.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleHealthChannelsTest,
    "PinkCab.Vehicle.Health.ComponentChannels",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleHealthChannelsTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState State;
    FPinkCabVehicleHealthService Service;

    const EPinkCabVehicleHealthChannel Channels[] = {
        EPinkCabVehicleHealthChannel::Wheel,
        EPinkCabVehicleHealthChannel::Tire,
        EPinkCabVehicleHealthChannel::Alignment,
        EPinkCabVehicleHealthChannel::Suspension,
        EPinkCabVehicleHealthChannel::Brake,
        EPinkCabVehicleHealthChannel::Clutch,
        EPinkCabVehicleHealthChannel::Gearbox,
        EPinkCabVehicleHealthChannel::Door,
        EPinkCabVehicleHealthChannel::Lamp
    };

    for (EPinkCabVehicleHealthChannel Channel : Channels)
    {
        State = FPinkCabVehicleHealthState();
        TestTrue(TEXT("synthetic functional hit applies"),
            Service.ApplyHit(State, FPinkCabVehicleHitEvent(Channel, 0.25f, false)));
        TestEqual(TEXT("channel health decreases deterministically"), State.GetHealth(Channel), 0.75f);
        TestEqual(TEXT("one functional mutation recorded"), State.GetFunctionalDamageSerial(), 1u);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabAirCooledHealthChannelsTest,
    "PinkCab.Vehicle.Health.AirCooledChannels",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabAirCooledHealthChannelsTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState State;
    FPinkCabVehicleHealthService Service;
    const EPinkCabVehicleHealthChannel Channels[] = {
        EPinkCabVehicleHealthChannel::EngineOil,
        EPinkCabVehicleHealthChannel::EngineFan,
        EPinkCabVehicleHealthChannel::OilCooler,
        EPinkCabVehicleHealthChannel::Airflow
    };

    for (EPinkCabVehicleHealthChannel Channel : Channels)
    {
        State = FPinkCabVehicleHealthState();
        TestTrue(TEXT("air-cooled hit applies"),
            Service.ApplyHit(State, FPinkCabVehicleHitEvent(Channel, 0.4f, false)));
        TestEqual(TEXT("air-cooled channel is independent"), State.GetHealth(Channel), 0.6f);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleTerminalCapabilityTest,
    "PinkCab.Vehicle.Health.TerminalCapability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleTerminalCapabilityTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthService Service;
    FPinkCabVehicleHealthState State;

    TestFalse(TEXT("healthy vehicle is not terminal"), Service.IsTerminal(State));
    Service.ApplyHit(State, FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::Lamp, 1.0f, false));
    Service.ApplyHit(State, FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::Door, 1.0f, false));
    TestFalse(TEXT("lamp/door loss is not universal-HP death"), Service.IsTerminal(State));

    Service.ApplyHit(State, FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::Brake, 1.0f, false));
    TestTrue(TEXT("loss of braking capability is terminal"), Service.IsTerminal(State));

    State = FPinkCabVehicleHealthState();
    Service.ApplyHit(State, FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::Gearbox, 1.0f, false));
    TestFalse(TEXT("broken gearbox removes drive capability"),
        Service.HasCapability(State, EPinkCabVehicleCapability::Drive));
    TestTrue(TEXT("broken gearbox is terminal for self-propelled taxi operation"),
        Service.IsTerminal(State));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleCosmeticHitTest,
    "PinkCab.Vehicle.Health.CosmeticHit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleCosmeticHitTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState State;
    FPinkCabVehicleHealthService Service;
    const uint32 Before = State.GetFunctionalDamageSerial();

    TestTrue(TEXT("cosmetic event is accepted as known nonfunctional damage"),
        Service.ApplyHit(State, FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::CosmeticBody, 1.0f, true)));
    TestEqual(TEXT("cosmetic event does not mutate functional health serial"),
        State.GetFunctionalDamageSerial(), Before);
    TestEqual(TEXT("wheel remains healthy"), State.GetHealth(EPinkCabVehicleHealthChannel::Wheel), 1.0f);
    TestFalse(TEXT("cosmetic event cannot terminally disable vehicle"), Service.IsTerminal(State));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleSyntheticHitMappingTest,
    "PinkCab.Vehicle.Health.SyntheticHitMapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleSyntheticHitMappingTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthService Service;
    FPinkCabVehicleHitEvent Event;
    TestTrue(TEXT("known synthetic wheel zone resolves"),
        Service.ResolveSyntheticHitZone(FName(TEXT("FrontLeftWheel")), 0.3f, Event));
    TestEqual(TEXT("wheel zone maps to wheel channel"), Event.Channel, EPinkCabVehicleHealthChannel::Wheel);
    TestEqual(TEXT("severity survives mapping"), Event.Severity, 0.3f);

    TestFalse(TEXT("unknown zone is explicit failure"),
        Service.ResolveSyntheticHitZone(FName(TEXT("UnknownZone")), 0.5f, Event));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchSlipHeatTest,
    "PinkCab.Vehicle.Health.Drivetrain.ClutchSlipHeat",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchSlipHeatTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabDrivetrainCondition Condition;
    FPinkCabDrivetrainConditionInput Input;
    Input.DeltaSeconds = 1.0f;
    Input.bEngineRunning = true;
    Input.EngineRpm = 3500.0f;
    Input.ExpectedCoupledRpm = 1500.0f;
    Input.SpeedKmh = 25.0f;
    Input.Throttle = 0.6f;
    Input.ClutchCoupling = 0.5f;
    Input.EngagedGear = 2;

    Condition.Step(Input, Health);
    const float Hot = Health.GetClutchTemperature01();
    TestTrue(TEXT("partial clutch with rpm mismatch creates heat"), Hot > 0.0f);
    TestTrue(TEXT("sustained slip causes causal clutch wear"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Clutch) < 1.0f);

    Input.ClutchCoupling = 0.0f;
    Input.EngineRpm = 5000.0f;
    Condition.Step(Input, Health);
    TestTrue(TEXT("fully disengaged clutch cannot add slip heat"),
        Health.GetClutchTemperature01() <= Hot);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabBrakeHeatCausalityTest,
    "PinkCab.Vehicle.Health.Drivetrain.BrakeHeatCausality",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabBrakeHeatCausalityTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabDrivetrainCondition Condition;
    FPinkCabDrivetrainConditionInput Input;
    Input.DeltaSeconds = 1.0f;
    Input.SpeedKmh = 60.0f;
    Input.Brake = 0.8f;
    Condition.Step(Input, Health);
    const float Hot = Health.GetBrakeTemperature01();
    TestTrue(TEXT("braking rotating wheels creates heat"), Hot > 0.0f);

    Input.SpeedKmh = 0.0f;
    Input.Brake = 0.0f;
    Input.Handbrake = 1.0f;
    Input.Throttle = 1.0f;
    Input.ClutchCoupling = 0.0f;
    Condition.Step(Input, Health);
    TestTrue(TEXT("free-rev + static handbrake does not add brake heat"),
        Health.GetBrakeTemperature01() <= Hot);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGearboxConflictWearTest,
    "PinkCab.Vehicle.Health.Drivetrain.GearboxConflictWear",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGearboxConflictWearTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabDrivetrainCondition Condition;
    const float Before = Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox);
    Condition.RecordGearEvent(EPinkCabGearEngagementResult::GrindRefused, Health);
    TestTrue(TEXT("grind causes gearbox damage"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox) < Before);

    const float AfterGrind = Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox);
    Condition.RecordGearEvent(EPinkCabGearEngagementResult::ClutchDisengagedAccepted, Health);
    TestEqual(TEXT("clean shift has no arbitrary wear"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox), AfterGrind);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCoupledStallTest,
    "PinkCab.Vehicle.Health.Drivetrain.CoupledStall",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCoupledStallTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabDrivetrainCondition Condition;
    FPinkCabDrivetrainConditionInput Input;
    Input.DeltaSeconds = 0.1f;
    Input.bEngineRunning = true;
    Input.EngineRpm = 500.0f;
    Input.SpeedKmh = 0.5f;
    Input.Brake = 0.8f;
    Input.Throttle = 0.0f;
    Input.ClutchCoupling = 1.0f;
    Input.EngagedGear = 1;
    TestTrue(TEXT("coupled low-rpm braking requests stall"), Condition.Step(Input, Health).bShouldStall);

    Input.ClutchCoupling = 0.0f;
    TestFalse(TEXT("disengaged clutch prevents drivetrain stall"), Condition.Step(Input, Health).bShouldStall);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabBrakeFadeTest,
    "PinkCab.Vehicle.Health.Drivetrain.BrakeFade",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabBrakeFadeTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabDrivetrainCondition Condition;
    FPinkCabDrivetrainConditionInput Input;
    Input.DeltaSeconds = 1.0f;
    Input.SpeedKmh = 100.0f;
    Input.Brake = 1.0f;
    for (int32 Index = 0; Index < 50; ++Index)
    {
        Condition.Step(Input, Health);
    }
    TestTrue(TEXT("sustained hard braking raises temperature"),
        Health.GetBrakeTemperature01() > 0.65f);
    TestTrue(TEXT("hot brakes fade but do not become hidden ABS"),
        Condition.GetBrakeEffectiveness(Health) < 1.0f && Condition.GetBrakeEffectiveness(Health) > 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabHealthyThirtyMinuteDriveTest,
    "PinkCab.Vehicle.Health.Drivetrain.HealthyThirtyMinutes",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabHealthyThirtyMinuteDriveTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabDrivetrainCondition Condition;
    FPinkCabDrivetrainConditionInput Input;
    Input.DeltaSeconds = 1.0f;
    Input.bEngineRunning = true;
    Input.EngineRpm = 2600.0f;
    Input.ExpectedCoupledRpm = 2600.0f;
    Input.SpeedKmh = 55.0f;
    Input.Throttle = 0.28f;
    Input.ClutchCoupling = 1.0f;
    Input.EngagedGear = 3;

    for (int32 Second = 0; Second < 1800; ++Second)
    {
        Input.Brake = (Second % 90) < 4 ? 0.25f : 0.0f;
        Condition.Step(Input, Health);
    }

    TestTrue(TEXT("normal driving keeps clutch healthy"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Clutch) > 0.99f);
    TestTrue(TEXT("normal driving keeps gearbox healthy"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox) > 0.99f);
    TestTrue(TEXT("normal driving keeps service brake healthy"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Brake) > 0.99f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabBrokenDrivetrainCapacityTest,
    "PinkCab.Vehicle.Health.Drivetrain.BrokenCapacity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabBrokenDrivetrainCapacityTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabDrivetrainCondition Condition;
    FPinkCabDrivetrainConditionInput Input;
    Input.DeltaSeconds = 0.1f;
    Input.bEngineRunning = true;
    Input.EngineRpm = 2500.0f;
    Input.ExpectedCoupledRpm = 2500.0f;
    Input.SpeedKmh = 40.0f;
    Input.Throttle = 0.5f;
    Input.ClutchCoupling = 1.0f;
    Input.EngagedGear = 3;

    Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Gearbox, 1.0f);
    const FPinkCabDrivetrainConditionOutput Output = Condition.Step(Input, Health);
    TestEqual(TEXT("destroyed gearbox transmits no drive torque"),
        Output.DrivetrainTorqueCapacity, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleHealthBindingTest,
    "PinkCab.Vehicle.Health.Binding.CanonicalOwner",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleHealthBindingTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthBinding Binding;
    FPinkCabVehicleHealthState External;
    External.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.20f);

    Binding.Bind(External);
    TestTrue(TEXT("binding reports canonical owner"), Binding.IsBound());
    TestEqual(TEXT("binding reads restored external state"),
        Binding.Get().GetHealth(EPinkCabVehicleHealthChannel::Brake), 0.80f);

    Binding.GetMutable().ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Clutch, 0.10f);
    TestEqual(TEXT("runtime mutation reaches external persistence owner"),
        External.GetHealth(EPinkCabVehicleHealthChannel::Clutch), 0.90f);

    Binding.UnbindPreservingState();
    TestFalse(TEXT("unbind releases external owner"), Binding.IsBound());
    TestEqual(TEXT("fallback keeps last canonical state"),
        Binding.Get().GetHealth(EPinkCabVehicleHealthChannel::Clutch), 0.90f);
    return true;
}


#endif
