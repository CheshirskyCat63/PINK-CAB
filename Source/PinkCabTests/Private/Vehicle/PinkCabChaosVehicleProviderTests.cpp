#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosEngineAdapter.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabThrottleResponse.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosProviderControlMappingTest,
    "PinkCab.Vehicle.ChaosBaseline.Provider.ControlMapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosProviderControlMappingTest::RunTest(const FString& Parameters)
{
    UPinkCabChaosVehicleMovementComponent* Movement = NewObject<UPinkCabChaosVehicleMovementComponent>();
    FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal)
        .ApplyToMovement(*Movement);
    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);

    FPinkCabVehicleControlState Controls;
    Controls.SetSteering(0.35f);
    Controls.SetThrottle(0.72f);
    Controls.SetBrake(0.18f);
    Controls.SetClutch(0.64f);
    Controls.SetHandbrake(0.80f);

    const float EngineThrottle = FPinkCabThrottleResponse::ToEngineThrottle(0.72f);
    Controls.SetResolvedEngineActuation(true, EngineThrottle, EngineThrottle, 200.0f, 100.0f);
    Controls.SetDriveline(2, 1, 0.36f);
    TestTrue(TEXT("provider accepts normalized controls"), Provider.ApplyControls(Controls));
    TestEqual(TEXT("Chaos vehicle-space steering preserves semantic right-positive"),
        Movement->GetSteeringInput(), 0.35f);
    TestEqual(TEXT("pedal linkage response reaches Chaos"),
        Movement->GetThrottleInput(), FPinkCabChaosEngineAdapter::ToChaosThrottleInput(EngineThrottle));
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
    TestEqual(TEXT("semantic telemetry reports the engaged first gear"), Telemetry.CurrentGear, 1);
    TestEqual(TEXT("semantic telemetry preserves the distinct second-gear request"), Telemetry.TargetGear, 2);
    TestEqual(TEXT("native diagnostic gear remains neutral"), Telemetry.NativeCurrentGear, 0);
    TestEqual(TEXT("native diagnostic target remains neutral"), Telemetry.NativeTargetGear, 0);
    FPinkCabChaosVehicleDynamicsProvider Reader(Movement);
    FPinkCabVehicleTelemetry SharedTelemetry;
    TestTrue(TEXT("a separate provider reads the same movement telemetry"), Reader.ReadTelemetry(SharedTelemetry));
    TestEqual(TEXT("separate provider observes the authoritative engaged gear"), SharedTelemetry.CurrentGear, 1);
    TestEqual(TEXT("separate provider observes the authoritative requested gear"), SharedTelemetry.TargetGear, 2);
    TestEqual(TEXT("authoritative requested drive retains engaged first"),
        Movement->GetPendingPinkCabDrivelineCommand().EngagedGear, 1);
    TestEqual(TEXT("authoritative coupling preserves continuous command"),
        Movement->GetPendingPinkCabDrivelineCommand().ClutchCoupling01, 0.36f);
    TestEqual(TEXT("telemetry wheel slot count mirrors Chaos"), Telemetry.Wheels.Num(), Movement->GetNumWheels());
    Controls.SetDriveline(-1, -1, 0.36f);
    TestTrue(TEXT("provider accepts authoritative reverse command"), Provider.ApplyControls(Controls));
    TestTrue(TEXT("separate provider reads reverse telemetry"), Reader.ReadTelemetry(SharedTelemetry));
    TestEqual(TEXT("reverse engaged gear survives native neutral transmission"), SharedTelemetry.CurrentGear, -1);
    TestEqual(TEXT("reverse request survives native neutral transmission"), SharedTelemetry.TargetGear, -1);
    TestEqual(TEXT("reverse still keeps the native diagnostic transmission neutral"), SharedTelemetry.NativeCurrentGear, 0);
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
    UPinkCabChaosVehicleMovementComponent* Movement =
        NewObject<UPinkCabChaosVehicleMovementComponent>();
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
    UPinkCabChaosVehicleMovementComponent* Movement =
        NewObject<UPinkCabChaosVehicleMovementComponent>();
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
    TestEqual(TEXT("partial coupling keeps native transmission neutral"),
        Movement->GetTargetGear(), 0);
    TestEqual(TEXT("partial coupling disables the legacy external torque path"),
        Partial.ExternalRearDriveTorquePerWheelNm, 0.0f);
    const FPinkCabChaosDrivelineCommand PartialCommand =
        Movement->GetPendingPinkCabDrivelineCommand();
    TestEqual(TEXT("partial coupling uses authoritative first gear"), PartialCommand.EngagedGear, 1);
    TestEqual(TEXT("partial coupling reaches the solver without threshold rounding"),
        PartialCommand.ClutchCoupling01, 0.999f);

    FPinkCabVehicleControlState Full;
    Full.SetThrottle(0.25f);
    Full.SetDriveline(1, 1, 1.0f);
    Full.SetDrivetrainTorqueCapacity(1.0f);
    TestTrue(TEXT("full coupling is accepted"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Full, Provider));
    TestEqual(TEXT("full coupling keeps the native transmission neutral"),
        Movement->GetTargetGear(), 0);
    const FPinkCabChaosDrivelineCommand FullCommand =
        Movement->GetPendingPinkCabDrivelineCommand();
    TestEqual(TEXT("full coupling keeps the same authoritative gear"),
        FullCommand.EngagedGear, PartialCommand.EngagedGear);
    TestEqual(TEXT("full coupling keeps the same effective ratio"),
        FullCommand.EffectiveGearRatio, PartialCommand.EffectiveGearRatio);
    TestEqual(TEXT("full coupling keeps the same throttle demand"),
        FullCommand.HealthClampedControlThrottle01, PartialCommand.HealthClampedControlThrottle01);
    TestEqual(TEXT("full coupling reaches the same solver at one"),
        FullCommand.ClutchCoupling01, 1.0f);
    TestEqual(TEXT("full coupling also disables the legacy external torque path"),
        Full.ExternalRearDriveTorquePerWheelNm, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosProviderRejectsLegacyMovementTest,
    "PinkCab.Vehicle.ChaosBaseline.Provider.RejectsLegacyMovement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosProviderRejectsLegacyMovementTest::RunTest(const FString& Parameters)
{
    UChaosWheeledVehicleMovementComponent* LegacyMovement =
        NewObject<UChaosWheeledVehicleMovementComponent>();
    FPinkCabChaosVehicleDynamicsProvider Provider(LegacyMovement);
    FPinkCabVehicleControlState Controls;
    Controls.SetThrottle(1.0f);
    Controls.SetDriveline(1, 1, 1.0f);
    TestFalse(TEXT("production provider refuses fallback to the legacy native drivetrain"),
        Provider.ApplyControls(Controls));
    FPinkCabCockpitState Cockpit;
    Cockpit.StartEngine();
    TestFalse(TEXT("cockpit bridge refuses a movement without the P02 adapter"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *LegacyMovement, Controls, Provider));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosBridgeHealthPermissionTest,
    "PinkCab.Vehicle.ChaosBaseline.Provider.EngineHealthPermission",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosBridgeHealthPermissionTest::RunTest(const FString& Parameters)
{
    UPinkCabChaosVehicleMovementComponent* Movement =
        NewObject<UPinkCabChaosVehicleMovementComponent>();
    FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal)
        .ApplyToMovement(*Movement);
    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
    FPinkCabCockpitState Cockpit;
    Cockpit.StartEngine();
    FPinkCabVehicleControlState Controls;
    Controls.SetThrottle(1.0f);
    Controls.SetDriveline(1, 1, 0.5f);

    for (const float Coupling : {0.0f, 0.5f, 1.0f})
    {
        Controls.SetDriveline(1, 1, Coupling);
        TestTrue(TEXT("health-disabled engine command reaches the physical adapter"),
            FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider, false));
        TestFalse(TEXT("health permission overrides running ignition at every coupling"),
            Movement->GetPendingPinkCabDrivelineCommand().bCombustionAllowed);
        TestEqual(TEXT("health-disabled engine has no resolved combustion torque"),
            Controls.GetAvailableEngineTorqueNm(), 0.0f);
        TestEqual(TEXT("health-disabled engine has no native throttle"), Movement->GetThrottleInput(), 0.0f);
        TestTrue(TEXT("health failure retains the mechanical coast/back-drive path"),
            Movement->bMechanicalSimEnabled);
        TestEqual(TEXT("health failure cannot silently disengage the selected gear"),
            Movement->GetPendingPinkCabDrivelineCommand().EngagedGear, 1);
    }

    TestTrue(TEXT("restored health is accepted by the same bridge"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider, true));
    TestTrue(TEXT("running restored engine permits combustion"),
        Movement->GetPendingPinkCabDrivelineCommand().bCombustionAllowed);
    TestTrue(TEXT("restored engine responds to the explicit driver demand"),
        Movement->GetThrottleInput() > 0.0f);
    return true;
}

#endif
