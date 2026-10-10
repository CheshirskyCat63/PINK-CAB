#pragma once

#include "CoreMinimal.h"

// Profile data only: retaining these values does not claim physical clutch actuation.
struct PINKCABVEHICLE_API FPinkCabClutchDrivelineConfig
{
    // Effective engine rotational inertia used only for the equal/opposite
    // clutch reaction impulse. Runtime wiring must source this explicitly
    // from the accepted engine profile; it is never hidden inside the solver.
    float EngineEffectiveInertia = 0.0f;

    // Maximum clutch torque at full engagement and healthy capacity.
    // This is an authored/calibrated physical parameter, not a launch helper.
    float MaxClutchTorqueNm = 0.0f;

    // Time horizon over which an unconstrained clutch attempts to remove slip.
    // Capacity still limits the actual torque, so this does not force a lock.
    float SynchronizationTimeSeconds = 0.0f;

    // Diagnostic/state threshold only. It never changes torque authority.
    float LockedSlipRpm = 0.0f;

    bool IsValid() const;
};
