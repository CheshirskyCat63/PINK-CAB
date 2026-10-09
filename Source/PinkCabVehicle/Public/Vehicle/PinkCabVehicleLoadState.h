#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Vehicle/PinkCabTatraProfile.h"

struct FPinkCabVehicleMassProperties;

struct PINKCABVEHICLE_API FPinkCabVehicleLoadItem
{
    FPinkCabVehicleLoadItem();
    FPinkCabVehicleLoadItem(float InMassKg, float InLongitudinalCm);
    FPinkCabVehicleLoadItem(float InMassKg, const FVector& InCenterCm);

    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
    float LateralCm = 0.0f;
    float VerticalCm = FPinkCabTatraProfile::DefaultPassengerHeightCm;
};

struct PINKCABVEHICLE_API FPinkCabVehicleLoadState
{
    void SetFuelMassKg(float InMassKg, float InLongitudinalCm = 0.0f);
    void SetFuelMassKg(float InMassKg, const FVector& InCenterCm);
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
    bool TryGetMassProperties(const FPinkCabTatraProfile& Profile,
        FPinkCabVehicleMassProperties& Out) const;

private:
    friend class FPinkCabVehicleSnapshotCodec;
    friend class FPinkCabVehicleStateSnapshotCodec;

    float FuelMassKg = 0.0f;
    float FuelLongitudinalCm = 0.0f;
    float FuelLateralCm = 0.0f;
    float FuelVerticalCm = FPinkCabTatraProfile::DefaultFuelHeightCm;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    TArray<FPinkCabVehicleLoadItem> Passengers;
    TArray<FPinkCabVehicleLoadItem, TInlineAllocator<5>> FarePassengers;
    FPinkCabStableId FarePassengerGroupId;
    bool bFarePassengerGroupActive = false;
};
