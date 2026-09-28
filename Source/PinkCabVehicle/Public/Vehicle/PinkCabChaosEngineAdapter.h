#pragma once

#include "CoreMinimal.h"

// UE 5.8 Chaos squares ThrottleInput inside
// UChaosWheeledVehicleSimulation::ApplyInput before feeding FSimpleEngineSim.
// PINK CAB owns the authoritative post-health/post-limiter engine throttle.
// This adapter maps that authoritative value onto Chaos' square-law input so
// Chaos consumes exactly the same throttle result as the external clutch path.
struct PINKCABVEHICLE_API FPinkCabChaosEngineAdapter
{
    static float ToChaosThrottleInput(const float AuthoritativeThrottle01)
    {
        return FMath::Sqrt(FMath::Clamp(AuthoritativeThrottle01, 0.0f, 1.0f));
    }
};
