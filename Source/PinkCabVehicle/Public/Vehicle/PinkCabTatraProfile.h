#pragma once

#include "CoreMinimal.h"

struct PINKCABVEHICLE_API FPinkCabTatraProfile
{
    static FPinkCabTatraProfile Canonical();

    float GetFullFuelVehicleMassKg() const { return BaseVehicleMassKg + FullFuelMassKg; }
    float GetReferenceCrewMassKg() const
    {
        return GetFullFuelVehicleMassKg() + HeroineMassKg + DaughterMassKg;
    }

    static constexpr int MassDistributionVersion = 2;
    static constexpr float DefaultFuelHeightCm = 32.0f;
    static constexpr float DefaultPassengerHeightCm = 73.5247f;
    float RearAssemblyMassKg = 0.0f;
    FVector RearAssemblyCenterCm = FVector::ZeroVector;
    FVector RearAssemblySizeCm = FVector::ZeroVector;
    float ChassisCenterHeightCm = 0.0f;
    FVector ChassisSizeCm = FVector::ZeroVector;
    float CrewHalfTrackCm = 0.0f;
    float CrewCenterHeightCm = 0.0f;
    FVector OccupantSizeCm = FVector::ZeroVector;
    FVector FuelSizeCm = FVector::ZeroVector;
    float BaseLongitudinalCm = 0.0f;
    float HeroineLongitudinalCm = 0.0f;
    float DaughterLongitudinalCm = 0.0f;
    float BaseVehicleMassKg = 0.0f;
    float FullFuelMassKg = 0.0f;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    float DeclaredMaxFixtureKg = 0.0f;
};
