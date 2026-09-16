#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosLoadBridge.h"
#include "Vehicle/PinkCabTatraProfile.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleHealthService.h"
#include "Vehicle/PinkCabVehicleLoadState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosLoadBridgeTest,
    "PinkCab.Vehicle.LiveState.LoadToChaos",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosLoadBridgeTest::RunTest(const FString& Parameters)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    FPinkCabVehicleLoadState Load;
    Load.SetFuelMassKg(100.0f, 0.0f);
    Load.SetCrew(Profile.HeroineMassKg, Profile.DaughterMassKg);
    UChaosWheeledVehicleMovementComponent* Movement = NewObject<UChaosWheeledVehicleMovementComponent>();
    TestTrue(TEXT("canonical load applies to Chaos"), FPinkCabChaosLoadBridge::Apply(Load, Profile, *Movement));
    TestEqual(TEXT("full fuel crew mass reaches live Chaos"), Movement->Mass, 1657.0f);
    TestTrue(TEXT("load owns longitudinal COM override"), Movement->bEnableCenterOfMassOverride);

    Load.AddPassenger(FPinkCabVehicleLoadItem(90.0f, -120.0f));
    TestTrue(TEXT("changed load reapplies to Chaos"), FPinkCabChaosLoadBridge::Apply(Load, Profile, *Movement));
    TestEqual(TEXT("passenger mass reaches live Chaos"), Movement->Mass, 1747.0f);
    TestTrue(TEXT("rear passenger shifts live COM rearward"), Movement->CenterOfMassOverride.X < 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleHealthControlGateTest,
    "PinkCab.Vehicle.LiveState.HealthControlGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleHealthControlGateTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthService Service;
    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleControlState Controls;
    Controls.SetSteering(0.7f);
    Controls.SetThrottle(0.8f);
    Controls.SetBrake(0.6f);
    TestTrue(TEXT("brake channel can be terminally damaged"),
        Service.ApplyHit(Health, FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::Brake, 1.0f, false)));
    Service.ApplyCapabilitiesToControls(Health, Controls);
    TestEqual(TEXT("lost brake capability cannot issue brake command"), Controls.Brake, 0.0f);
    TestEqual(TEXT("unrelated throttle remains available"), Controls.Throttle, 0.8f);

    FPinkCabVehicleHealthState EngineHealth;
    Service.ApplyHit(EngineHealth, FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::EngineOil, 1.0f, false));
    Controls.SetThrottle(1.0f);
    Service.ApplyCapabilitiesToControls(EngineHealth, Controls);
    TestEqual(TEXT("lost engine capability blocks throttle"), Controls.Throttle, 0.0f);
    return true;
}

#endif
