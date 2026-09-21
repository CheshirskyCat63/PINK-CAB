#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabThrottleResponse.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

bool FPinkCabChaosCockpitBridge::Apply(
    const FPinkCabCockpitState& Cockpit,
    UChaosWheeledVehicleMovementComponent& Movement,
    FPinkCabVehicleControlState& Controls,
    FPinkCabChaosVehicleDynamicsProvider& Provider)
{
    const bool bEngineRunning =
        Cockpit.GetIgnitionState() == EPinkCabIgnitionState::Running;
    Movement.EnableMechanicalSim(bEngineRunning);
    Movement.SetUseAutomaticGears(false);

    // Chaos has no public clutch axis. PINK CAB keeps Chaos as the tire/contact
    // solver while using its normal transmission at full coupling and additive
    // rear-wheel drive torque during partial clutch transfer.
    constexpr float FullyCoupledThreshold = 1.0f;
    const bool bFullyCoupled =
        Controls.ClutchCoupling >= FullyCoupledThreshold
        && Controls.EngagedGear != 0;
    Movement.SetTargetGear(bFullyCoupled ? Controls.EngagedGear : 0, true);

    float ExternalRearDriveTorquePerWheelNm = 0.0f;
    if (bEngineRunning
        && Controls.EngagedGear != 0
        && Controls.ClutchCoupling > KINDA_SMALL_NUMBER
        && Controls.ClutchCoupling < FullyCoupledThreshold)
    {
        const float EngineRpm = Movement.GetEngineRotationSpeed();
        const float NormalizedTorque =
            Movement.EngineSetup.TorqueCurve.GetRichCurveConst()->Eval(EngineRpm);
        const float EngineTorqueNm =
            Movement.EngineSetup.MaxTorque * FMath::Max(NormalizedTorque, 0.0f);
        const float GearRatio =
            Movement.TransmissionSetup.GetGearRatio(Controls.EngagedGear);

        // Chaos exposes no public clutch axis, so partial coupling is authored here.
        // The wheel-torque bridge must preserve the driver's actual pedal command.
        // No idle governor, launch helper, or minimum throttle is allowed to synthesize
        // drivetrain torque when the pedal target is zero.
        const float EffectiveThrottle =
            FPinkCabThrottleResponse::ToEngineThrottle(Controls.Throttle);
        const float AxleTorqueNm =
            EngineTorqueNm
            * GearRatio
            * Movement.TransmissionSetup.TransmissionEfficiency
            * EffectiveThrottle
            * Controls.ClutchCoupling
            * Controls.DrivetrainTorqueCapacity;
        ExternalRearDriveTorquePerWheelNm = AxleTorqueNm * 0.5f;
    }
    Controls.SetExternalRearDriveTorquePerWheel(ExternalRearDriveTorquePerWheelNm);

    return Provider.ApplyControls(Controls);
}
