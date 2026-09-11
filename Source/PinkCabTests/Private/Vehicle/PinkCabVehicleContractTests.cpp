#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"
#include "Vehicle/PinkCabVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabTatraProfile.h"
#include "Vehicle/PinkCabVehicleLoadState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatraMassFixturesTest,
    "PinkCab.Vehicle.Contracts.MassFixtures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatraMassFixturesTest::RunTest(const FString& Parameters)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    TestEqual(TEXT("base vehicle mass"), Profile.BaseVehicleMassKg, 1450.0f);
    TestEqual(TEXT("full fuel vehicle mass"), Profile.GetFullFuelVehicleMassKg(), 1550.0f);
    TestEqual(TEXT("heroine mass"), Profile.HeroineMassKg, 58.0f);
    TestEqual(TEXT("daughter mass"), Profile.DaughterMassKg, 49.0f);
    TestEqual(TEXT("full-fuel crew reference"), Profile.GetReferenceCrewMassKg(), 1657.0f);
    TestEqual(TEXT("declared maximum fixture"), Profile.DeclaredMaxFixtureKg, 2107.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleLoadStateTest,
    "PinkCab.Vehicle.Contracts.LoadAndCg",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleLoadStateTest::RunTest(const FString& Parameters)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    FPinkCabVehicleLoadState Load;
    Load.SetFuelMassKg(100.0f);
    Load.SetCrew(Profile.HeroineMassKg, Profile.DaughterMassKg);
    TestEqual(TEXT("crew + full fuel reaches reference fixture"), Load.GetTotalMassKg(Profile), 1657.0f);

    const float CrewCg = Load.GetLongitudinalCgInputCm(Profile);
    Load.AddPassenger(FPinkCabVehicleLoadItem(90.0f, -120.0f));
    TestEqual(TEXT("passenger mass is authoritative"), Load.GetTotalMassKg(Profile), 1747.0f);
    TestTrue(TEXT("rear passenger shifts CG input rearward"), Load.GetLongitudinalCgInputCm(Profile) < CrewCg);

    Load.AddPassenger(FPinkCabVehicleLoadItem(360.0f, -80.0f));
    TestEqual(TEXT("declared maximum fixture can be represented exactly"), Load.GetTotalMassKg(Profile), 2107.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleNormalizedContractsTest,
    "PinkCab.Vehicle.Contracts.NormalizedControlTelemetry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleNormalizedContractsTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlState Controls;
    Controls.SetSteering(2.0f);
    Controls.SetThrottle(0.75f);
    Controls.SetBrake(-1.0f);
    Controls.SetClutch(1.5f);
    Controls.SetHandbrake(0.4f);
    TestEqual(TEXT("steering signed normalized"), Controls.Steering, 1.0f);
    TestEqual(TEXT("throttle normalized"), Controls.Throttle, 0.75f);
    TestEqual(TEXT("brake normalized"), Controls.Brake, 0.0f);
    TestEqual(TEXT("clutch normalized"), Controls.Clutch, 1.0f);
    TestEqual(TEXT("handbrake normalized"), Controls.Handbrake, 0.4f);

    FPinkCabVehicleTelemetry Telemetry;
    Telemetry.SpeedKmh = 123.0f;
    Telemetry.EngineRpm = 4100.0f;
    Telemetry.NormalizedSteering = -0.2f;
    TestEqual(TEXT("telemetry is vendor-neutral scalar data"), Telemetry.SpeedKmh, 123.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleNoProviderTest,
    "PinkCab.Vehicle.Contracts.NoProvider",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleNoProviderTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleDynamicsProviderHandle Handle;
    TestEqual(TEXT("pre-FGear provider is explicitly absent"), Handle.GetState(), EPinkCabVehicleDynamicsProviderState::NoProvider);
    TestFalse(TEXT("no provider cannot apply controls"), Handle.ApplyControls(FPinkCabVehicleControlState()));
    FPinkCabVehicleTelemetry OutTelemetry;
    TestFalse(TEXT("no provider cannot read road telemetry"), Handle.ReadTelemetry(OutTelemetry));
    return true;
}

#endif
