#include "Vehicle/PinkCabChaosCockpitBridge.h"

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

    Controls.SetHandbrake(Cockpit.GetHandbrakeAmount());
    return Provider.ApplyControls(Controls);
}
