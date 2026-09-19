#pragma once

#include "CoreMinimal.h"

// Mechanical pedal/linkage response, not an assist: zero pedal stays zero and
// full pedal stays full. The concave curve gives the heavy car useful low-pedal
// torque while preserving the driver's exact command for instruments/telemetry.
struct FPinkCabThrottleResponse
{
    static float ToEngineThrottle(const float DriverThrottle)
    {
        const float Clamped = FMath::Clamp(DriverThrottle, 0.0f, 1.0f);
        return Clamped <= 0.0f ? 0.0f : FMath::Pow(Clamped, 0.55f);
    }
};
