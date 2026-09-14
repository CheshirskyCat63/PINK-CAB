#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleLoadState.h"

struct FPinkCabVehicleHealthSnapshot
{
    TArray<float> ChannelHealth;
    uint32 FunctionalDamageSerial = 0;
};

struct FPinkCabVehicleLoadItemSnapshot
{
    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};

struct FPinkCabVehicleLoadSnapshot
{
    float FuelMassKg = 0.0f;
    float FuelLongitudinalCm = 0.0f;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    TArray<FPinkCabVehicleLoadItemSnapshot> Passengers;
    TArray<FPinkCabVehicleLoadItemSnapshot> FarePassengers;
    FString FarePassengerGroupId;
    bool bFarePassengerGroupActive = false;
};

struct FPinkCabVehicleSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;
    int32 SchemaVersion = CurrentSchemaVersion;
    FPinkCabVehicleHealthSnapshot Health;
    FPinkCabVehicleLoadSnapshot Load;
};

class FPinkCabVehicleSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabVehicleHealthState& Health,
        const FPinkCabVehicleLoadState& Load,
        FPinkCabVehicleSnapshot& OutSnapshot)
    {
        FPinkCabVehicleSnapshot Snapshot;
        for (int32 Index = 0; Index < static_cast<int32>(EPinkCabVehicleHealthChannel::Count); ++Index)
        {
            Snapshot.Health.ChannelHealth.Add(Health.Health[Index]);
        }
        Snapshot.Health.FunctionalDamageSerial = Health.FunctionalDamageSerial;
        Snapshot.Load.FuelMassKg = Load.FuelMassKg;
        Snapshot.Load.FuelLongitudinalCm = Load.FuelLongitudinalCm;
        Snapshot.Load.HeroineMassKg = Load.HeroineMassKg;
        Snapshot.Load.DaughterMassKg = Load.DaughterMassKg;
        for (const FPinkCabVehicleLoadItem& Item : Load.Passengers)
        {
            Snapshot.Load.Passengers.Add({Item.MassKg, Item.LongitudinalCm});
        }
        for (const FPinkCabVehicleLoadItem& Item : Load.FarePassengers)
        {
            Snapshot.Load.FarePassengers.Add({Item.MassKg, Item.LongitudinalCm});
        }
        Snapshot.Load.FarePassengerGroupId = Load.FarePassengerGroupId.Serialize();
        Snapshot.Load.bFarePassengerGroupActive = Load.bFarePassengerGroupActive;
        if (!Validate(Snapshot))
        {
            return false;
        }
        OutSnapshot = MoveTemp(Snapshot);
        return true;
    }
    static bool Restore(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthState& OutHealth,
        FPinkCabVehicleLoadState& OutLoad)
    {
        if (!Validate(Snapshot))
        {
            return false;
        }

        FPinkCabVehicleHealthState Health;
        for (int32 Index = 0; Index < Snapshot.Health.ChannelHealth.Num(); ++Index)
        {
            Health.Health[Index] = Snapshot.Health.ChannelHealth[Index];
        }
        Health.FunctionalDamageSerial = Snapshot.Health.FunctionalDamageSerial;

        FPinkCabVehicleLoadState Load;
        Load.FuelMassKg = Snapshot.Load.FuelMassKg;
        Load.FuelLongitudinalCm = Snapshot.Load.FuelLongitudinalCm;
        Load.HeroineMassKg = Snapshot.Load.HeroineMassKg;
        Load.DaughterMassKg = Snapshot.Load.DaughterMassKg;
        for (const FPinkCabVehicleLoadItemSnapshot& Saved : Snapshot.Load.Passengers)
        {
            Load.Passengers.Add(FPinkCabVehicleLoadItem(Saved.MassKg, Saved.LongitudinalCm));
        }
        for (const FPinkCabVehicleLoadItemSnapshot& Saved : Snapshot.Load.FarePassengers)
        {
            Load.FarePassengers.Add(FPinkCabVehicleLoadItem(Saved.MassKg, Saved.LongitudinalCm));
        }
        if (Snapshot.Load.bFarePassengerGroupActive)
        {
            Load.FarePassengerGroupId = FPinkCabStableId(Snapshot.Load.FarePassengerGroupId);
            Load.bFarePassengerGroupActive = true;
        }

        OutHealth = MoveTemp(Health);
        OutLoad = MoveTemp(Load);
        return true;
    }

private:
    static bool ValidateItem(const FPinkCabVehicleLoadItemSnapshot& Item)
    {
        return FMath::IsFinite(Item.MassKg)
            && FMath::IsFinite(Item.LongitudinalCm)
            && Item.MassKg > 0.0f;
    }

    static bool Validate(const FPinkCabVehicleSnapshot& Snapshot)
    {
        if (Snapshot.SchemaVersion != FPinkCabVehicleSnapshot::CurrentSchemaVersion
            || Snapshot.Health.ChannelHealth.Num()
                != static_cast<int32>(EPinkCabVehicleHealthChannel::Count))
        {
            return false;
        }
        for (const float Value : Snapshot.Health.ChannelHealth)
        {
            if (!FMath::IsFinite(Value) || Value < 0.0f || Value > 1.0f)
            {
                return false;
            }
        }
        if (!FMath::IsFinite(Snapshot.Load.FuelMassKg)
            || !FMath::IsFinite(Snapshot.Load.FuelLongitudinalCm)
            || !FMath::IsFinite(Snapshot.Load.HeroineMassKg)
            || !FMath::IsFinite(Snapshot.Load.DaughterMassKg)
            || Snapshot.Load.FuelMassKg < 0.0f
            || Snapshot.Load.HeroineMassKg < 0.0f
            || Snapshot.Load.DaughterMassKg < 0.0f)
        {
            return false;
        }        for (const FPinkCabVehicleLoadItemSnapshot& Item : Snapshot.Load.Passengers)
        {
            if (!ValidateItem(Item))
            {
                return false;
            }
        }
        for (const FPinkCabVehicleLoadItemSnapshot& Item : Snapshot.Load.FarePassengers)
        {
            if (!ValidateItem(Item))
            {
                return false;
            }
        }

        if (Snapshot.Load.bFarePassengerGroupActive)
        {
            FPinkCabStableId GroupId;
            if (!FPinkCabStableId::TryParse(Snapshot.Load.FarePassengerGroupId, GroupId)
                || Snapshot.Load.FarePassengers.Num() < 1
                || Snapshot.Load.FarePassengers.Num() > 5)
            {
                return false;
            }
        }
        else if (!Snapshot.Load.FarePassengerGroupId.IsEmpty()
            || !Snapshot.Load.FarePassengers.IsEmpty())
        {
            return false;
        }
        return true;
    }
};
