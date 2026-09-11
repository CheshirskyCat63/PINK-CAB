#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabTatraProfile.h"

struct FPinkCabVehicleLoadItem
{
    FPinkCabVehicleLoadItem() = default;
    FPinkCabVehicleLoadItem(float InMassKg, float InLongitudinalCm)
        : MassKg(FMath::Max(0.0f, InMassKg))
        , LongitudinalCm(InLongitudinalCm)
    {
    }

    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};

struct FPinkCabVehicleLoadState
{
    void SetFuelMassKg(float InMassKg, float InLongitudinalCm = 0.0f)
    {
        FuelMassKg = FMath::Max(0.0f, InMassKg);
        FuelLongitudinalCm = InLongitudinalCm;
    }

    void SetCrew(float InHeroineMassKg, float InDaughterMassKg)
    {
        HeroineMassKg = FMath::Max(0.0f, InHeroineMassKg);
        DaughterMassKg = FMath::Max(0.0f, InDaughterMassKg);
    }

    void AddPassenger(const FPinkCabVehicleLoadItem& Item)
    {
        Passengers.Add(Item);
    }

    float GetTotalMassKg(const FPinkCabTatraProfile& Profile) const
    {
        float Total = Profile.BaseVehicleMassKg + FuelMassKg + HeroineMassKg + DaughterMassKg;
        for (const FPinkCabVehicleLoadItem& Item : Passengers)
        {
            Total += Item.MassKg;
        }
        return Total;
    }

    float GetLongitudinalCgInputCm(const FPinkCabTatraProfile& Profile) const
    {
        const float TotalMassKg = GetTotalMassKg(Profile);
        if (TotalMassKg <= KINDA_SMALL_NUMBER)
        {
            return 0.0f;
        }

        float WeightedCmKg = FuelMassKg * FuelLongitudinalCm;
        for (const FPinkCabVehicleLoadItem& Item : Passengers)
        {
            WeightedCmKg += Item.MassKg * Item.LongitudinalCm;
        }
        return WeightedCmKg / TotalMassKg;
    }

private:
    float FuelMassKg = 0.0f;
    float FuelLongitudinalCm = 0.0f;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    TArray<FPinkCabVehicleLoadItem> Passengers;
};
