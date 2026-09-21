#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabThrottleResponse.h"

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
    TestEqual(TEXT("Chaos vehicle-space steering inverts semantic right-positive exactly once"),
        Movement->GetSteeringInput(), -0.35f);
    TestEqual(TEXT("pedal linkage response reaches Chaos"),
        Movement->GetThrottleInput(), FPinkCabThrottleResponse::ToEngineThrottle(0.72f));
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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosBridgeZeroThrottleNoSyntheticTorqueTest,
    "PinkCab.Vehicle.ChaosBaseline.Provider.ZeroThrottleNoSyntheticDriveTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosBridgeZeroThrottleNoSyntheticTorqueTest::RunTest(const FString& Parameters)
{
    UChaosWheeledVehicleMovementComponent* Movement =
        NewObject<UChaosWheeledVehicleMovementComponent>();
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToMovement(*Movement);

    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
    FPinkCabCockpitState Cockpit;
    Cockpit.StartEngine();

    FPinkCabVehicleControlState Controls;
    Controls.SetThrottle(0.0f);
    Controls.SetDriveline(1, 1, 0.50f);
    Controls.SetDrivetrainTorqueCapacity(1.0f);

    TestTrue(TEXT("bridge accepts configured zero-throttle partial-clutch state"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider));
    TestEqual(TEXT("zero throttle must not fabricate rear drive torque"),
        Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);
    TestEqual(TEXT("provider still receives the real zero pedal command"),
        Provider.GetLastControls().Throttle, 0.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosBridgeFullCouplingBoundaryTest,
    "PinkCab.Vehicle.ChaosBaseline.Provider.FullCouplingBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosBridgeFullCouplingBoundaryTest::RunTest(const FString& Parameters)
{
    UChaosWheeledVehicleMovementComponent* Movement =
        NewObject<UChaosWheeledVehicleMovementComponent>();
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToMovement(*Movement);

    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
    FPinkCabCockpitState Cockpit;
    Cockpit.StartEngine();

    FPinkCabVehicleControlState Partial;
    Partial.SetThrottle(0.25f);
    Partial.SetDriveline(1, 1, 0.999f);
    Partial.SetDrivetrainTorqueCapacity(1.0f);
    TestTrue(TEXT("99.9 percent coupling is accepted"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Partial, Provider));
    TestEqual(TEXT("partial clutch keeps Chaos transmission neutral until physically full coupling"),
        Movement->GetTargetGear(), 0);
    TestTrue(TEXT("partial clutch still carries authored continuous external torque"),
        FMath::Abs(Partial.ExternalRearDriveTorquePerWheelNm) > KINDA_SMALL_NUMBER);

    FPinkCabVehicleControlState Full;
    Full.SetThrottle(0.25f);
    Full.SetDriveline(1, 1, 1.0f);
    Full.SetDrivetrainTorqueCapacity(1.0f);
    TestTrue(TEXT("full coupling is accepted"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Full, Provider));
    TestEqual(TEXT("only full coupling hands engaged gear to Chaos transmission"),
        Movement->GetTargetGear(), 1);
    TestEqual(TEXT("full coupling stops the partial-clutch external torque path"),
        Full.ExternalRearDriveTorquePerWheelNm, 0.0f);
    return true;
}

#endif
