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
    Controls.SetSteering(-0.35f);
    Controls.SetThrottle(0.72f);
    Controls.SetBrake(0.18f);
    Controls.SetClutch(0.64f);
    Controls.SetHandbrake(0.80f);

    TestTrue(TEXT("provider accepts normalized controls"), Provider.ApplyControls(Controls));
    TestEqual(TEXT("steering reaches Chaos"), Movement->GetSteeringInput(), -0.35f);
    TestEqual(TEXT("throttle reaches Chaos"), Movement->GetThrottleInput(), 0.72f);
    TestEqual(TEXT("brake reaches Chaos"), Movement->GetBrakeInput(), 0.18f);
    TestTrue(TEXT("handbrake threshold reaches Chaos"), Movement->GetHandbrakeInput());
    FPinkCabVehicleTelemetry Telemetry;
    TestTrue(TEXT("provider returns telemetry"), Provider.ReadTelemetry(Telemetry));
    TestEqual(TEXT("telemetry echoes steering command"), Telemetry.NormalizedSteering, -0.35f);
    TestEqual(TEXT("telemetry echoes throttle command"), Telemetry.NormalizedThrottle, 0.72f);
    TestEqual(TEXT("telemetry preserves clutch command without claiming Chaos clutch actuation"), Telemetry.NormalizedClutch, 0.64f);
    TestEqual(TEXT("telemetry echoes handbrake command"), Telemetry.NormalizedHandbrake, 0.80f);
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