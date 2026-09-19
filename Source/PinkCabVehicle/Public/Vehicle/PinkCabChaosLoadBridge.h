#pragma once

#include "CoreMinimal.h"

class UChaosWheeledVehicleMovementComponent;
struct FPinkCabTatraProfile;
struct FPinkCabVehicleLoadState;

struct PINKCABVEHICLE_API FPinkCabChaosLoadBridge
{
    static bool Apply(
        const FPinkCabVehicleLoadState& Load,
        const FPinkCabTatraProfile& Profile,
        UChaosWheeledVehicleMovementComponent& Movement);
};
