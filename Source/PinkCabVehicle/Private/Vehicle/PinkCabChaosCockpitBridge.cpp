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
    Movement.EnableMechanicalSim(bCombustionAllowed);
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

    constexpr float FullyCoupledThreshold = 1.0f;
    const bool bFullyCoupled =
        Controls.ClutchCoupling >= FullyCoupledThreshold
        && Controls.EngagedGear != 0;
    Movement.SetTargetGear(bFullyCoupled ? Controls.EngagedGear : 0, true);

    float ExternalRearDriveTorquePerWheelNm = 0.0f;
    if (Controls.IsCombustionAllowed()
        && Controls.EngagedGear != 0
        && Controls.ClutchCoupling > KINDA_SMALL_NUMBER
        && Controls.ClutchCoupling < FullyCoupledThreshold)
    {
        const float GearRatio =
            Movement.TransmissionSetup.GetGearRatio(Controls.EngagedGear);
        const float AxleTorqueNm =
            Controls.GetAvailableEngineTorqueNm()
            * GearRatio
            * Movement.TransmissionSetup.TransmissionEfficiency
            * Controls.ClutchCoupling
            * Controls.DrivetrainTorqueCapacity;
        ExternalRearDriveTorquePerWheelNm = AxleTorqueNm * 0.5f;
    }
    Controls.SetExternalRearDriveTorquePerWheel(
        ExternalRearDriveTorquePerWheelNm);

    return Provider.ApplyControls(Controls);
}
