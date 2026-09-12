#pragma once

#include "CoreMinimal.h"

class UChaosWheeledVehicleMovementComponent;
class FPinkCabChaosVehicleDynamicsProvider;
struct FPinkCabCockpitState;
struct FPinkCabVehicleControlState;

struct PINKCAB_API FPinkCabChaosCockpitBridge
{
    static bool Apply(
        const FPinkCabCockpitState& Cockpit,
        UChaosWheeledVehicleMovementComponent& Movement,
        FPinkCabVehicleControlState& Controls,
        FPinkCabChaosVehicleDynamicsProvider& Provider);
};
