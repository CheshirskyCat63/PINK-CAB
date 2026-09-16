#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleDamageProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleDamageProfileTest,
    "PinkCab.Vehicle.Health.AuthoredDamageProfile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleDamageProfileTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleDamageProfile Profile(TEXT("PinkCab.Damage.Test"));
    TestTrue(TEXT("authored brake hydraulic zone registers"), Profile.TryAddZone(
        TEXT("BrakeHydraulic"), EPinkCabVehicleHealthChannel::BrakeHydraulic, 0.40f));
    TestFalse(TEXT("duplicate authored zone rejected"), Profile.TryAddZone(
        TEXT("BrakeHydraulic"), EPinkCabVehicleHealthChannel::Brake, 0.10f));

    FPinkCabVehicleHitEvent Event;
    TestFalse(TEXT("below threshold stays cosmetic/no functional event"),
        Profile.ResolveFunctionalHit(TEXT("BrakeHydraulic"), 0.40f, Event));
    TestTrue(TEXT("above authored threshold resolves"),
        Profile.ResolveFunctionalHit(TEXT("BrakeHydraulic"), 0.55f, Event));
    TestEqual(TEXT("profile owns channel mapping"),
        Event.Channel, EPinkCabVehicleHealthChannel::BrakeHydraulic);
    TestEqual(TEXT("collision severity is preserved"), Event.Severity, 0.55f);
    return true;
}

#endif
