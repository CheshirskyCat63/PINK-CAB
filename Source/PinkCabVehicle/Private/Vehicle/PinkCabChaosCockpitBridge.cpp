#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
float ResolveTorqueCurveNm(
    const UChaosWheeledVehicleMovementComponent& Movement,
    const float EngineRpm)
{
    const float Normalized =
        Movement.EngineSetup.TorqueCurve.GetRichCurveConst()->Eval(EngineRpm);
    return Movement.EngineSetup.MaxTorque * FMath::Max(Normalized, 0.0f);
}
}

bool FPinkCabChaosCockpitBridge::Apply(
    const FPinkCabCockpitState& Cockpit,
    UChaosWheeledVehicleMovementComponent& Movement,
    FPinkCabVehicleControlState& Controls,
    FPinkCabChaosVehicleDynamicsProvider& Provider)
{
    const bool bCombustionAllowed =
        Cockpit.GetIgnitionState() == EPinkCabIgnitionState::Running;
    // Mechanical simulation is the physical driveline path and must remain
    // alive while ignition is Off/Stalled. Combustion permission is carried
    // independently so key-off can coast and mechanically back-drive without
    // producing fuel torque.
    Movement.EnableMechanicalSim(true);
    Movement.SetUseAutomaticGears(false);

    const float EngineRpm = Movement.GetEngineRotationSpeed();
    const float EngineTorqueCurveNm =
        ResolveTorqueCurveNm(Movement, EngineRpm);
    FPinkCabEngineActuationInput ActuationInput;
    ActuationInput.bCombustionAllowed = bCombustionAllowed;
    ActuationInput.HealthClampedControlThrottle01 = Controls.Throttle;
    ActuationInput.EngineRpm = EngineRpm;
    ActuationInput.MaxRpm = Movement.EngineSetup.MaxRPM;
    ActuationInput.EngineTorqueCurveNm = EngineTorqueCurveNm;
    const FPinkCabEngineActuationResult Actuation =
        FPinkCabEngineActuationResolver::Resolve(ActuationInput);
    Controls.SetResolvedEngineActuation(
        Actuation.bCombustionAllowed,
        Actuation.EngineThrottlePreLimiter01,
        Actuation.EngineThrottleFinal01,
        EngineTorqueCurveNm,
        Actuation.RequestedEngineTorqueAfterLimiterHealthNm);

    // PHY-009 owns one driveline solver for 0..1 coupling. Chaos' simple
    // transmission has no clutch model and must stay neutral; otherwise 1.0
    // would silently switch back to a different RPM/torque solver.
    Movement.SetTargetGear(0, true);

    // Compatibility transport only. No production torque is allowed through
    // the legacy external-partial path after P02.
    Controls.SetExternalRearDriveTorquePerWheel(0.0f);

    return Provider.ApplyControls(Controls);
}
