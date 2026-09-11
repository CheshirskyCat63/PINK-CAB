#pragma once

struct FPinkCabTatraProfile
{
    static FPinkCabTatraProfile Canonical()
    {
        FPinkCabTatraProfile Result;
        Result.BaseVehicleMassKg = 1450.0f;
        Result.FullFuelMassKg = 100.0f;
        Result.HeroineMassKg = 58.0f;
        Result.DaughterMassKg = 49.0f;
        Result.DeclaredMaxFixtureKg = 2107.0f;
        return Result;
    }

    float GetFullFuelVehicleMassKg() const { return BaseVehicleMassKg + FullFuelMassKg; }
    float GetReferenceCrewMassKg() const
    {
        return GetFullFuelVehicleMassKg() + HeroineMassKg + DaughterMassKg;
    }

    float BaseVehicleMassKg = 0.0f;
    float FullFuelMassKg = 0.0f;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    float DeclaredMaxFixtureKg = 0.0f;
};
