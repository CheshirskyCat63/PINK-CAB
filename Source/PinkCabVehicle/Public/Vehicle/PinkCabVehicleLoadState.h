#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Vehicle/PinkCabTatraProfile.h"

struct PINKCABVEHICLE_API FPinkCabVehicleLoadItem
{
    FPinkCabVehicleLoadItem();
    FPinkCabVehicleLoadItem(float InMassKg, float InLongitudinalCm);

    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};

struct PINKCABVEHICLE_API FPinkCabVehicleLoadState
{
    void SetFuelMassKg(float InMassKg, float InLongitudinalCm = 0.0f);
    float GetFuelMassKg() const;
    void SetCrew(float InHeroineMassKg, float InDaughterMassKg);
    void AddPassenger(const FPinkCabVehicleLoadItem& Item);
    bool TrySetFarePassengerGroup(
        const FPinkCabStableId& GroupId,
        TConstArrayView<FPinkCabVehicleLoadItem> Items);
    bool RemoveFarePassengerGroup(const FPinkCabStableId& GroupId);
    bool HasFarePassengerGroup(const FPinkCabStableId& GroupId) const;
    float GetTotalMassKg(const FPinkCabTatraProfile& Profile) const;
    float GetLongitudinalCgInputCm(const FPinkCabTatraProfile& Profile) const;

private:
    friend class FPinkCabVehicleSnapshotCodec;
    friend class FPinkCabVehicleStateSnapshotCodec;

    float FuelMassKg = 0.0f;
    float FuelLongitudinalCm = 0.0f;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    TArray<FPinkCabVehicleLoadItem> Passengers;
    TArray<FPinkCabVehicleLoadItem, TInlineAllocator<5>> FarePassengers;
    FPinkCabStableId FarePassengerGroupId;
    bool bFarePassengerGroupActive = false;
};
