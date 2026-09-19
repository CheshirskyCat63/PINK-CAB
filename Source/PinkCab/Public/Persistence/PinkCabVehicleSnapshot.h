#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"

using FPinkCabVehicleHealthSnapshot = FPinkCabVehicleHealthStateSnapshot;
using FPinkCabVehicleLoadItemSnapshot = FPinkCabVehicleLoadItemStateSnapshot;
using FPinkCabVehicleLoadSnapshot = FPinkCabVehicleLoadStateSnapshot;

struct FPinkCabVehicleSnapshot
{
    static constexpr int32 LegacySchemaVersion = 1;
    static constexpr int32 DivergentSchemaVersion = 2;
    static constexpr int32 CurrentSchemaVersion = 3;

    static constexpr int32 LegacySchema1HealthChannelCount = 12;
    static constexpr int32 RuntimeSchema2HealthChannelCount = 14;
    static constexpr int32 PresentationSchema2HealthChannelCount = 16;

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
        FPinkCabVehicleStateSnapshot StateSnapshot;
        if (!FPinkCabVehicleStateSnapshotCodec::Capture(Health, Load, StateSnapshot))
        {
            return false;
        }
        FPinkCabVehicleSnapshot Snapshot;
        Snapshot.Health = StateSnapshot.Health;
        Snapshot.Load = StateSnapshot.Load;
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
        FPinkCabVehicleStateSnapshot StateSnapshot;
        StateSnapshot.Load = Snapshot.Load;
        MigrateHealth(Snapshot, StateSnapshot.Health);
        StateSnapshot.Health.FunctionalDamageSerial = Snapshot.Health.FunctionalDamageSerial;
        return FPinkCabVehicleStateSnapshotCodec::Restore(
            StateSnapshot, OutHealth, OutLoad);
    }

private:
    static void MigrateHealth(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthStateSnapshot& Health)
    {
        const TArray<float>& Saved = Snapshot.Health.ChannelHealth;
        Health.ChannelHealth.Init(
            1.0f, static_cast<int32>(EPinkCabVehicleHealthChannel::Count));

        if (Snapshot.SchemaVersion == FPinkCabVehicleSnapshot::LegacySchemaVersion)
        {
            // v1:
            // Wheel,Tire,Alignment,Suspension,Brake,Door,Lamp,EngineOil,
            // EngineFan,OilCooler,Airflow,CosmeticBody.
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Wheel)] = Saved[0];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Tire)] = Saved[1];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Alignment)] = Saved[2];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Suspension)] = Saved[3];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Brake)] = Saved[4];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Door)] = Saved[5];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Lamp)] = Saved[6];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::EngineOil)] = Saved[7];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::EngineFan)] = Saved[8];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::OilCooler)] = Saved[9];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Airflow)] = Saved[10];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::CosmeticBody)] = Saved[11];
            Health.ClutchTemperature01 = 0.0f;
            Health.BrakeTemperature01 = 0.0f;
            return;
        }

        if (Snapshot.SchemaVersion == FPinkCabVehicleSnapshot::DivergentSchemaVersion
            && Saved.Num() == FPinkCabVehicleSnapshot::RuntimeSchema2HealthChannelCount)
        {
            // Runtime v2:
            // Wheel,Tire,Alignment,Suspension,Brake,Clutch,Gearbox,Door,Lamp,
            // EngineOil,EngineFan,OilCooler,Airflow,CosmeticBody.
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Wheel)] = Saved[0];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Tire)] = Saved[1];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Alignment)] = Saved[2];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Suspension)] = Saved[3];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Brake)] = Saved[4];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Clutch)] = Saved[5];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Gearbox)] = Saved[6];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Door)] = Saved[7];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Lamp)] = Saved[8];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::EngineOil)] = Saved[9];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::EngineFan)] = Saved[10];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::OilCooler)] = Saved[11];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Airflow)] = Saved[12];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::CosmeticBody)] = Saved[13];
            Health.ClutchTemperature01 = Snapshot.Health.ClutchTemperature01;
            Health.BrakeTemperature01 = Snapshot.Health.BrakeTemperature01;
            return;
        }

        if (Snapshot.SchemaVersion == FPinkCabVehicleSnapshot::DivergentSchemaVersion
            && Saved.Num() == FPinkCabVehicleSnapshot::PresentationSchema2HealthChannelCount)
        {
            // CD-855 presentation/damage v2:
            // Wheel,Tire,Alignment,Suspension,Brake,BrakeHeat,BrakeHydraulic,
            // Door,Lamp,Glass,EngineOil,EngineHead,EngineFan,OilCooler,Airflow,
            // CosmeticBody.
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Wheel)] = Saved[0];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Tire)] = Saved[1];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Alignment)] = Saved[2];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Suspension)] = Saved[3];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Brake)] = Saved[4];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::BrakeHeat)] = Saved[5];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::BrakeHydraulic)] = Saved[6];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Door)] = Saved[7];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Lamp)] = Saved[8];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Glass)] = Saved[9];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::EngineOil)] = Saved[10];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::EngineHead)] = Saved[11];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::EngineFan)] = Saved[12];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::OilCooler)] = Saved[13];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::Airflow)] = Saved[14];
            Health.ChannelHealth[static_cast<int32>(EPinkCabVehicleHealthChannel::CosmeticBody)] = Saved[15];
            Health.ClutchTemperature01 = 0.0f;
            Health.BrakeTemperature01 = 0.0f;
            return;
        }

        // Current v3 layout exactly matches the unified enum.
        for (int32 Index = 0; Index < Saved.Num(); ++Index)
        {
            Health.ChannelHealth[Index] = Saved[Index];
        }
        Health.ClutchTemperature01 = Snapshot.Health.ClutchTemperature01;
        Health.BrakeTemperature01 = Snapshot.Health.BrakeTemperature01;
    }

    static bool ValidateItem(const FPinkCabVehicleLoadItemSnapshot& Item)
    {
        return FMath::IsFinite(Item.MassKg)
            && FMath::IsFinite(Item.LongitudinalCm)
            && Item.MassKg > 0.0f;
    }

    static bool Validate(const FPinkCabVehicleSnapshot& Snapshot)
    {
        const int32 Count = Snapshot.Health.ChannelHealth.Num();
        const bool bLegacyV1 =
            Snapshot.SchemaVersion == FPinkCabVehicleSnapshot::LegacySchemaVersion
            && Count == FPinkCabVehicleSnapshot::LegacySchema1HealthChannelCount;
        const bool bRuntimeV2 =
            Snapshot.SchemaVersion == FPinkCabVehicleSnapshot::DivergentSchemaVersion
            && Count == FPinkCabVehicleSnapshot::RuntimeSchema2HealthChannelCount;
        const bool bPresentationV2 =
            Snapshot.SchemaVersion == FPinkCabVehicleSnapshot::DivergentSchemaVersion
            && Count == FPinkCabVehicleSnapshot::PresentationSchema2HealthChannelCount;
        const bool bCurrent =
            Snapshot.SchemaVersion == FPinkCabVehicleSnapshot::CurrentSchemaVersion
            && Count == static_cast<int32>(EPinkCabVehicleHealthChannel::Count);

        if (!bLegacyV1 && !bRuntimeV2 && !bPresentationV2 && !bCurrent)
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

        const bool bHasThermalFields = bRuntimeV2 || bCurrent;
        if (bHasThermalFields
            && (!FMath::IsFinite(Snapshot.Health.ClutchTemperature01)
                || !FMath::IsFinite(Snapshot.Health.BrakeTemperature01)
                || Snapshot.Health.ClutchTemperature01 < 0.0f
                || Snapshot.Health.ClutchTemperature01 > 1.0f
                || Snapshot.Health.BrakeTemperature01 < 0.0f
                || Snapshot.Health.BrakeTemperature01 > 1.0f))
        {
            return false;
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
        }

        for (const FPinkCabVehicleLoadItemSnapshot& Item : Snapshot.Load.Passengers)
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
            if (!FPinkCabStableId::TryParse(
                    Snapshot.Load.FarePassengerGroupId,
                    GroupId)
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
