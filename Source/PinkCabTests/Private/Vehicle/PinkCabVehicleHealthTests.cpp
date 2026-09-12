#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleHitEvent.h"
#include "Vehicle/PinkCabVehicleHealthService.h"

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
#endif
