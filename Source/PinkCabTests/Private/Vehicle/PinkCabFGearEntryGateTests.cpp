#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleDynamicsProvider.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFGearProviderSlotGateTest,
    "PinkCab.Vehicle.FGearEntryGate.ProviderSlotEmpty",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFGearProviderSlotGateTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleDynamicsProviderHandle Provider;
    TestEqual(TEXT("pre-purchase provider slot must remain empty"),
        Provider.GetState(), EPinkCabVehicleDynamicsProviderState::NoProvider);

    FPinkCabVehicleControlState Controls;
    FPinkCabVehicleTelemetry Telemetry;
    TestFalse(TEXT("controls cannot reach production dynamics without provider"),
        Provider.ApplyControls(Controls));
    TestFalse(TEXT("telemetry cannot be fabricated without provider"),
        Provider.ReadTelemetry(Telemetry));
    return true;
}

#endif
