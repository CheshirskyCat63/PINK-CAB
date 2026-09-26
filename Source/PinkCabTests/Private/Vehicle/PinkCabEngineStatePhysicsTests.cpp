#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
FPinkCabEngineActuationInput ResolverInput(
    const bool bAllowed,
    const float Throttle,
    const float Rpm = 2500.0f)
{
    FPinkCabEngineActuationInput Input;
    Input.bCombustionAllowed = bAllowed;
    Input.HealthClampedControlThrottle01 = Throttle;
    Input.EngineRpm = Rpm;
    Input.MaxRpm = 8500.0f;
    Input.EngineTorqueCurveNm = 260.0f;
    return Input;
}

FPinkCabCockpitState StalledCockpit()
{
    FPinkCabCockpitState Cockpit;
    Cockpit.StartEngine();
    Cockpit.StallEngine();
    return Cockpit;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCombustionPermissionResolverTest,
    "PinkCab.Vehicle.Physics.EngineState.CombustionPermissionResolver",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCombustionPermissionResolverTest::RunTest(const FString& Parameters)
{
    for (const float Throttle : {0.0f, 0.25f, 1.0f})
    {
        const FPinkCabEngineActuationResult Off =
            FPinkCabEngineActuationResolver::Resolve(ResolverInput(false, Throttle));
        TestFalse(TEXT("permission is retained in result while off"), Off.bCombustionAllowed);
        TestEqual(TEXT("off final engine throttle is zero"), Off.EngineThrottleFinal01, 0.0f);
        TestEqual(TEXT("off available engine torque is zero"),
            Off.RequestedEngineTorqueAfterLimiterHealthNm, 0.0f);

        const FPinkCabEngineActuationResult Running =
            FPinkCabEngineActuationResolver::Resolve(ResolverInput(true, Throttle));
        TestTrue(TEXT("running permission is retained"), Running.bCombustionAllowed);
        if (Throttle > 0.0f)
        {
            TestTrue(TEXT("running demand survives below limiter"),
                Running.EngineThrottleFinal01 > 0.0f);
            TestTrue(TEXT("running demand has positive available torque"),
                Running.RequestedEngineTorqueAfterLimiterHealthNm > 0.0f);
        }
    }

    const FPinkCabEngineActuationResult Limited =
        FPinkCabEngineActuationResolver::Resolve(ResolverInput(true, 1.0f, 8500.0f));
    TestEqual(TEXT("limiter remains authoritative after permission"), Limited.EngineThrottleFinal01, 0.0f);
    TestEqual(TEXT("limiter also removes available combustion torque"),
        Limited.RequestedEngineTorqueAfterLimiterHealthNm, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCombustionPermissionBridgeMatrixTest,
    "PinkCab.Vehicle.Physics.EngineState.OffStalledMatrix",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCombustionPermissionBridgeMatrixTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);

    for (const EPinkCabIgnitionState State :
        {EPinkCabIgnitionState::Off, EPinkCabIgnitionState::Stalled})
    {
        for (const float Throttle : {0.0f, 0.25f, 1.0f})
        {
            for (const int32 Gear : {0, 1, -1})
            {
                for (const float Coupling : {0.0f, 0.50f, 1.0f})
                {
                    UChaosWheeledVehicleMovementComponent* Movement =
                        NewObject<UChaosWheeledVehicleMovementComponent>();
                    Profile.ApplyToMovement(*Movement);
                    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
                    FPinkCabCockpitState Cockpit =
                        State == EPinkCabIgnitionState::Stalled
                            ? StalledCockpit()
                            : FPinkCabCockpitState();

                    FPinkCabVehicleControlState Controls;
                    Controls.SetThrottle(Throttle);
                    Controls.SetDriveline(Gear, Gear, Coupling);
                    Controls.SetDrivetrainTorqueCapacity(1.0f);

                    TestTrue(TEXT("bridge applies off/stalled matrix cell"),
                        FPinkCabChaosCockpitBridge::Apply(
                            Cockpit, *Movement, Controls, Provider));
                    TestFalse(TEXT("off/stalled control state denies combustion"),
                        Controls.IsCombustionAllowed());
                    TestEqual(TEXT("off/stalled resolved final throttle is zero"),
                        Controls.GetResolvedEngineThrottle01(), 0.0f);
                    TestEqual(TEXT("off/stalled resolved available engine torque is zero"),
                        Controls.GetAvailableEngineTorqueNm(), 0.0f);
                    TestEqual(TEXT("off/stalled partial path torque is zero"),
                        Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);
                    TestEqual(TEXT("off/stalled Chaos throttle input is zero"),
                        Movement->GetThrottleInput(), 0.0f);
                }
            }
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCombustionPermissionRunningPathTest,
    "PinkCab.Vehicle.Physics.EngineState.RunningUsesSharedActuation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCombustionPermissionRunningPathTest::RunTest(const FString& Parameters)
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
    Partial.SetThrottle(0.50f);
    Partial.SetDriveline(1, 1, 0.50f);
    Partial.SetDrivetrainTorqueCapacity(1.0f);
    TestTrue(TEXT("running partial clutch applies"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Partial, Provider));
    TestTrue(TEXT("running state permits combustion"), Partial.IsCombustionAllowed());
    TestTrue(TEXT("running resolved throttle is positive"),
        Partial.GetResolvedEngineThrottle01() > 0.0f);
    TestTrue(TEXT("running available torque is positive"),
        Partial.GetAvailableEngineTorqueNm() > 0.0f);
    TestTrue(TEXT("partial clutch consumes shared available torque"),
        FMath::Abs(Partial.ExternalRearDriveTorquePerWheelNm) > KINDA_SMALL_NUMBER);
    TestEqual(TEXT("provider applies the exact resolved throttle"),
        Movement->GetThrottleInput(), Partial.GetResolvedEngineThrottle01());

    FPinkCabVehicleControlState Full;
    Full.SetThrottle(0.50f);
    Full.SetDriveline(1, 1, 1.0f);
    Full.SetDrivetrainTorqueCapacity(1.0f);
    TestTrue(TEXT("running full clutch applies"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Full, Provider));
    TestTrue(TEXT("full coupling keeps shared available torque visible"),
        Full.GetAvailableEngineTorqueNm() > 0.0f);
    TestEqual(TEXT("full coupling does not also inject external rear torque"),
        Full.ExternalRearDriveTorquePerWheelNm, 0.0f);
    TestEqual(TEXT("full coupling applies the same resolved throttle to Chaos"),
        Movement->GetThrottleInput(), Full.GetResolvedEngineThrottle01());
    return true;
}

#endif
