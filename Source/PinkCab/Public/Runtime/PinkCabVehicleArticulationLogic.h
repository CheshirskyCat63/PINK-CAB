#pragma once

#include "CoreMinimal.h"

struct PINKCAB_API FPinkCabVehicleArticulationState
{
    float Current = 0.0f;
    float Target = 0.0f;

    bool SetTarget(float InTarget);
    bool Toggle();
    bool Advance(float DeltaSeconds, float TravelSeconds);
};

PINKCAB_API FQuat PinkCabArticulationRotation(
    const FVector& LocalAxis,
    float OpenAngleDegrees,
    float Fraction);
