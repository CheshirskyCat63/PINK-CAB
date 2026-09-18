#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
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

    float GetFuelMassKg() const { return FuelMassKg; }

    void SetCrew(float InHeroineMassKg, float InDaughterMassKg)
    {
        HeroineMassKg = FMath::Max(0.0f, InHeroineMassKg);
        DaughterMassKg = FMath::Max(0.0f, InDaughterMassKg);
    }

    void AddPassenger(const FPinkCabVehicleLoadItem& Item)
    {
        Passengers.Add(Item);
    }

    bool TrySetFarePassengerGroup(
        const FPinkCabStableId& GroupId,
        TConstArrayView<FPinkCabVehicleLoadItem> Items)
    {
        if (bFarePassengerGroupActive || !GroupId.IsValid() || Items.IsEmpty() || Items.Num() > 5)
        {
            return false;
        }
        FarePassengerGroupId = GroupId;
        FarePassengers.Reset(Items.Num());
        FarePassengers.Append(Items.GetData(), Items.Num());
        bFarePassengerGroupActive = true;
        return true;
    }

    bool RemoveFarePassengerGroup(const FPinkCabStableId& GroupId)
    {
        if (!bFarePassengerGroupActive || GroupId != FarePassengerGroupId)
        {
            return false;
        }
        FarePassengers.Reset();
        FarePassengerGroupId = FPinkCabStableId();
        bFarePassengerGroupActive = false;
        return true;
    }

    bool HasFarePassengerGroup(const FPinkCabStableId& GroupId) const
    {
        return bFarePassengerGroupActive && GroupId == FarePassengerGroupId;
    }

    float GetTotalMassKg(const FPinkCabTatraProfile& Profile) const
    {
        float Total = Profile.BaseVehicleMassKg + FuelMassKg + HeroineMassKg + DaughterMassKg;
        for (const FPinkCabVehicleLoadItem& Item : Passengers)
        {
            Total += Item.MassKg;
        }
        for (const FPinkCabVehicleLoadItem& Item : FarePassengers)
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
        for (const FPinkCabVehicleLoadItem& Item : FarePassengers)
        {
            WeightedCmKg += Item.MassKg * Item.LongitudinalCm;
        }
        return WeightedCmKg / TotalMassKg;
    }

private:
    friend class FPinkCabVehicleSnapshotCodec;

    float FuelMassKg = 0.0f;
    float FuelLongitudinalCm = 0.0f;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    TArray<FPinkCabVehicleLoadItem> Passengers;
    TArray<FPinkCabVehicleLoadItem, TInlineAllocator<5>> FarePassengers;
    FPinkCabStableId FarePassengerGroupId;
    bool bFarePassengerGroupActive = false;
};
