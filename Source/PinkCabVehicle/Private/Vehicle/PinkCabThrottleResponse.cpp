#include "Vehicle/PinkCabThrottleResponse.h"

namespace
{
// P03 / PHY-015 mechanical pedal-linkage calibration.
// Keeps 0 -> 0 and 1 -> 1 while reducing the previous low-input amplification.
constexpr float PedalLinkageExponent = 0.75f;
}

float FPinkCabThrottleResponse::ToEngineThrottle(float DriverThrottle)
{
    const float Clamped = FMath::Clamp(DriverThrottle, 0.0f, 1.0f);
    return Clamped <= 0.0f ? 0.0f : FMath::Pow(Clamped, PedalLinkageExponent);
}
