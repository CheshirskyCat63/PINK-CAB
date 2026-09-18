#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosProviderControlMappingTest,
    "PinkCab.Vehicle.ChaosBaseline.Provider.ControlMapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosProviderControlMappingTest::RunTest(const FString& Parameters)
{
    UChaosWheeledVehicleMovementComponent* Movement = NewObject<UChaosWheeledVehicleMovementComponent>();
    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);

    FPinkCabVehicleControlState Controls;
    Controls.SetSteering(0.35f);
    Controls.SetThrottle(0.72f);
    Controls.SetBrake(0.18f);
    Controls.SetClutch(0.64f);
    Controls.SetHandbrake(0.80f);

    TestTrue(TEXT("provider accepts normalized controls"), Provider.ApplyControls(Controls));
    TestEqual(TEXT("semantic right-positive steering stays right-positive at Chaos provider boundary"), Movement->GetSteeringInput(), 0.35f);
    TestEqual(TEXT("throttle reaches Chaos"), Movement->GetThrottleInput(), 0.72f);
    TestEqual(TEXT("brake reaches Chaos"), Movement->GetBrakeInput(), 0.18f);
    TestFalse(TEXT("legacy bool handbrake path stays disabled"), Movement->GetHandbrakeInput());
    TestEqual(TEXT("provider retains continuous analog handbrake command"),
        Provider.GetLastControls().Handbrake, 0.80f);
    FPinkCabVehicleTelemetry Telemetry;
    TestTrue(TEXT("provider returns telemetry"), Provider.ReadTelemetry(Telemetry));
    TestEqual(TEXT("telemetry preserves semantic right-positive steering"), Telemetry.NormalizedSteering, 0.35f);
    TestEqual(TEXT("telemetry echoes throttle command"), Telemetry.NormalizedThrottle, 0.72f);
    TestEqual(TEXT("telemetry preserves clutch command without claiming Chaos clutch actuation"), Telemetry.NormalizedClutch, 0.64f);
    TestEqual(TEXT("telemetry echoes handbrake command"), Telemetry.NormalizedHandbrake, 0.80f);
    TestEqual(TEXT("telemetry mirrors current Chaos gear"), Telemetry.CurrentGear, Movement->GetCurrentGear());
    TestEqual(TEXT("telemetry wheel slot count mirrors Chaos"), Telemetry.Wheels.Num(), Movement->GetNumWheels());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosProviderNullGuardTest,
    "PinkCab.Vehicle.ChaosBaseline.Provider.NullGuard",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosProviderNullGuardTest::RunTest(const FString& Parameters)
{
    FPinkCabChaosVehicleDynamicsProvider Provider(nullptr);
    FPinkCabVehicleTelemetry Telemetry;
    TestFalse(TEXT("null provider rejects controls"), Provider.ApplyControls(FPinkCabVehicleControlState()));
    TestFalse(TEXT("null provider rejects telemetry"), Provider.ReadTelemetry(Telemetry));
    return true;
}

#endif
