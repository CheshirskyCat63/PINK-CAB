#include "Vehicle/PinkCabThrottleResponse.h"

float FPinkCabThrottleResponse::ToEngineThrottle(float DriverThrottle)
{
    const float Clamped = FMath::Clamp(DriverThrottle, 0.0f, 1.0f);
    return Clamped <= 0.0f ? 0.0f : FMath::Pow(Clamped, 0.55f);
}
