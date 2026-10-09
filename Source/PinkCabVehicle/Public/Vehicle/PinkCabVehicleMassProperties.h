#pragma once

#include "CoreMinimal.h"

// Immutable derived mass data in the accepted chassis frame (kg, cm, kg*cm^2).
// Chaos remains the dynamics solver; this contains no forces or time integration.
struct PINKCABVEHICLE_API FPinkCabVehicleMassProperties
{
    double MassKg = 0.0;
    FVector CenterCm = FVector::ZeroVector;
    FVector PrincipalInertiaKgCm2 = FVector::ZeroVector;
    FQuat PrincipalRotation = FQuat::Identity;

    bool IsValid() const;
};
