#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleControlRuntime.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabGearboxTypes.h"

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


FPinkCabVehicleTelemetry EngineStateTelemetry(
    const float SpeedKmh = 0.0f,
    const float EngineRpm = 925.0f)
{
    FPinkCabVehicleTelemetry Result;
    Result.SpeedKmh = SpeedKmh;
    Result.EngineRpm = EngineRpm;
    return Result;
}

FPinkCabVehicleControlTickInput EngineStateTick(
    const bool bClutch,
    const bool bBrake,
    const bool bThrottle,
    const int32 WheelSteps = 0)
{
    FPinkCabVehicleControlTickInput Input;
    Input.Frame = FPinkCabVehicleInputFrame::FromDigital(
        false, bClutch, bBrake, bThrottle);
    Input.WheelSteps = WheelSteps;
    Input.DeltaSeconds = 0.20f;
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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEngineStateTransitionCleanupTest,
    "PinkCab.Vehicle.Physics.EngineState.RestartClearsPropulsionDemand",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEngineStateTransitionCleanupTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;
    Cockpit.StartEngine();

    Runtime.Update(
        EngineStateTick(false, false, true, 10),
        EngineStateTelemetry(),
        Cockpit,
        Health);
    TestTrue(TEXT("precondition has authored throttle demand"),
        Runtime.GetThrottleTarget() > 0.0f);
    const float SteeringBefore = Runtime.GetSteeringCommand();
    Runtime.ForceGearState(1, 1, Cockpit);

    Cockpit.StallEngine();
    Runtime.ResetEngineTransition();
    TestEqual(TEXT("stall clears throttle target"), Runtime.GetThrottleTarget(), 0.0f);
    TestEqual(TEXT("stall clears current control throttle"),
        Runtime.GetControlState().Throttle, 0.0f);
    TestEqual(TEXT("stall clears stale external torque"),
        Runtime.GetControlState().ExternalRearDriveTorquePerWheelNm, 0.0f);
    TestFalse(TEXT("stall clears combustion permission"),
        Runtime.GetControlState().IsCombustionAllowed());
    TestEqual(TEXT("engine reset preserves engaged gear"),
        Runtime.GetEngagedGear(), 1);
    TestEqual(TEXT("engine reset preserves steering target"),
        Runtime.GetSteeringCommand(), SteeringBefore);

    TestTrue(TEXT("restart transitions Stalled to Running"), Cockpit.StartEngine());
    Runtime.ResetEngineTransition();
    Runtime.Update(
        EngineStateTick(false, false, true, 0),
        EngineStateTelemetry(),
        Cockpit,
        Health);
    TestEqual(TEXT("held E after restart cannot resurrect old dose"),
        Runtime.GetThrottleTarget(), 0.0f);
    TestEqual(TEXT("restart without fresh wheel dose has zero throttle"),
        Runtime.GetControlState().Throttle, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEngineStateTransitionSequenceTest,
    "PinkCab.Vehicle.Physics.EngineState.TransitionSequence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEngineStateTransitionSequenceTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitState Cockpit;
    TestEqual(TEXT("initial state is Off"),
        Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Off);
    TestTrue(TEXT("Off starts once"), Cockpit.StartEngine());
    TestEqual(TEXT("state becomes Running"),
        Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Running);
    TestFalse(TEXT("second start while Running is ignored"), Cockpit.StartEngine());
    Cockpit.StallEngine();
    TestEqual(TEXT("low-rpm failure enters Stalled"),
        Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Stalled);
    TestTrue(TEXT("Stalled restarts once"), Cockpit.StartEngine());
    TestEqual(TEXT("restart returns Running"),
        Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Running);
    Cockpit.StopEngine();
    TestEqual(TEXT("key-off returns Off"),
        Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Off);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEngineOffPreservesNonPropulsionControlsTest,
    "PinkCab.Vehicle.Physics.EngineState.OffPreservesCoastControls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEngineOffPreservesNonPropulsionControlsTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;
    Cockpit.StartEngine();
    Runtime.ForceGearState(3, 3, Cockpit);

    FPinkCabVehicleControlTickInput Tick =
        EngineStateTick(true, true, false, 0);
    Tick.SteeringMouseX = 300.0f;
    Runtime.Update(Tick, EngineStateTelemetry(45.0f, 2200.0f), Cockpit, Health);

    const float Steering = Runtime.GetControlState().Steering;
    const float Brake = Runtime.GetControlState().Brake;
    const float Clutch = Runtime.GetControlState().Clutch;
    const int32 Gear = Runtime.GetEngagedGear();

    Cockpit.StopEngine();
    Runtime.ResetEngineTransition();

    TestEqual(TEXT("key-off does not zero steering"),
        Runtime.GetControlState().Steering, Steering);
    TestEqual(TEXT("key-off does not zero service brake"),
        Runtime.GetControlState().Brake, Brake);
    TestEqual(TEXT("key-off does not zero clutch"),
        Runtime.GetControlState().Clutch, Clutch);
    TestEqual(TEXT("key-off does not force neutral"),
        Runtime.GetEngagedGear(), Gear);
    TestEqual(TEXT("key-off removes only positive throttle"),
        Runtime.GetControlState().Throttle, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWarmIdleProfileTest,
    "PinkCab.Vehicle.Physics.EngineState.WarmIdleProfile925",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWarmIdleProfileTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(
            EPinkCabCalibrationVariant::Nominal);
    TestTrue(TEXT("warm carb idle target is inside 900-950"),
        Profile.EngineIdleRpm.Value >= 900.0f
            && Profile.EngineIdleRpm.Value <= 950.0f);
    TestEqual(TEXT("calibration center is 925 rpm"),
        Profile.EngineIdleRpm.Value, 925.0f);

    UChaosWheeledVehicleMovementComponent* Movement =
        NewObject<UChaosWheeledVehicleMovementComponent>();
    Profile.ApplyToMovement(*Movement);
    TestEqual(TEXT("Chaos receives profile idle target"),
        Movement->EngineSetup.EngineIdleRPM, Profile.EngineIdleRpm.Value);

    const FPinkCabGearboxControllerConfig GearboxDefaults;
    TestEqual(TEXT("gearbox neutral rpm seed aligns until P02 centralization"),
        GearboxDefaults.IdleRpm, Profile.EngineIdleRpm.Value);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabLowRpmCoupledOverloadStallTest,
    "PinkCab.Vehicle.Physics.EngineState.LowRpmCoupledOverloadStalls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabLowRpmCoupledOverloadStallTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;

    TestTrue(TEXT("stall fixture starts engine"), Cockpit.StartEngine());
    Runtime.ForceGearState(1, 1, Cockpit);

    const FPinkCabVehicleControlOutput Output = Runtime.Update(
        EngineStateTick(false, false, false),
        EngineStateTelemetry(0.0f, 925.0f),
        Cockpit,
        Health);

    TestEqual(TEXT("stopping in gear at warm idle floor enters Stalled"),
        Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Stalled);
    TestEqual(TEXT("stall removes throttle command"),
        Output.Controls.Throttle, 0.0f);
    TestEqual(TEXT("stall preserves engaged gear for physical back-drive/coast"),
        Runtime.GetEngagedGear(), 1);
    return true;
}


#endif
