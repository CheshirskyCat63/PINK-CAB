#pragma once

#include "CoreMinimal.h"

struct PINKCAB_API FPinkCabVehicleArticulationState
{
    float Current = 0.0f;
    float Target = 0.0f;

    bool SetTarget(float InTarget)
    {
        if (!FMath::IsFinite(InTarget))
        {
            return false;
        }
        Target = FMath::Clamp(InTarget, 0.0f, 1.0f);
        return true;
    }

    bool Toggle()
    {
        Target = Target >= 0.5f ? 0.0f : 1.0f;
        return Target > 0.5f;
    }

    bool Advance(float DeltaSeconds, float TravelSeconds)
    {
        if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds < 0.0f
            || !FMath::IsFinite(TravelSeconds) || TravelSeconds <= KINDA_SMALL_NUMBER)
        {
            return false;
        }
        Current = FMath::FInterpConstantTo(
            Current,
            Target,
            DeltaSeconds,
            1.0f / TravelSeconds);
        Current = FMath::Clamp(Current, 0.0f, 1.0f);
        return true;
    }
};

inline FQuat PinkCabArticulationRotation(
    const FVector& LocalAxis,
    const float OpenAngleDegrees,
    const float Fraction)
{
    if (LocalAxis.ContainsNaN() || LocalAxis.IsNearlyZero()
        || !FMath::IsFinite(OpenAngleDegrees) || !FMath::IsFinite(Fraction))
    {
        return FQuat::Identity;
    }
    return FQuat(
        LocalAxis.GetSafeNormal(),
        FMath::DegreesToRadians(OpenAngleDegrees * FMath::Clamp(Fraction, 0.0f, 1.0f)));
}
