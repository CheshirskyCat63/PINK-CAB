#include "Vehicle/PinkCabVehicleInputResponse.h"

float FPinkCabVehicleInputResponse::StepAxis(
    float Current,
    float Target,
    float DeltaSeconds,
    float PressSeconds,
    float ReleaseSeconds)
{
    const float ClampedCurrent = FMath::Clamp(Current, 0.0f, 1.0f);
    const float ClampedTarget = FMath::Clamp(Target, 0.0f, 1.0f);
    if (DeltaSeconds <= 0.0f || FMath::IsNearlyEqual(ClampedCurrent, ClampedTarget))
    {
        return ClampedCurrent;
    }

    const float Duration = ClampedTarget > ClampedCurrent ? PressSeconds : ReleaseSeconds;
    const float SafeDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
    return FMath::Clamp(
        FMath::FInterpConstantTo(ClampedCurrent, ClampedTarget, DeltaSeconds, 1.0f / SafeDuration),
        FMath::Min(ClampedCurrent, ClampedTarget),
        FMath::Max(ClampedCurrent, ClampedTarget));
}
