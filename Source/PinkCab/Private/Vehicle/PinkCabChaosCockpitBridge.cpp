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
    Movement.SetUseAutomaticGears(false);

    // Chaos Wheeled Vehicles has no native clutch axis. FIRST EURO therefore uses a
    // bounded neutral-gate adapter: clutch pressure is still continuous, and the
    // selected 5+N+R cockpit gear is reconnected only when the release curve reaches zero.
    const int32 DrivelineGear = Controls.Clutch > KINDA_SMALL_NUMBER
        ? 0
        : Cockpit.GetSelectedGear();
    Movement.SetTargetGear(DrivelineGear, true);

    Controls.SetHandbrake(Cockpit.GetHandbrakeAmount());
    return Provider.ApplyControls(Controls);
}
