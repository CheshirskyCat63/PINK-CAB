#pragma once

#include "CoreMinimal.h"

// Mechanical pedal/linkage response, not an assist: zero pedal stays zero and
// full pedal stays full. The concave curve gives the heavy car useful low-pedal
// torque while preserving the driver's exact command for instruments/telemetry.
struct PINKCABVEHICLE_API FPinkCabThrottleResponse
{
    static float ToEngineThrottle(float DriverThrottle);
};
