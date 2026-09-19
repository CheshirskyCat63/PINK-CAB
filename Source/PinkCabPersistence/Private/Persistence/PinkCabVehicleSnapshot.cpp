#include "Persistence/PinkCabVehicleSnapshot.h"

bool FPinkCabVehicleSnapshotCodec::Capture(
    const FPinkCabVehicleHealthState& Health,
    const FPinkCabVehicleLoadState& Load,
    FPinkCabVehicleSnapshot& OutSnapshot)
{
    FPinkCabVehicleStateSnapshot StateSnapshot;
    if (!FPinkCabVehicleStateSnapshotCodec::Capture(
            Health,
            Load,
            StateSnapshot))
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

bool FPinkCabVehicleSnapshotCodec::Restore(
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
    StateSnapshot.Health.FunctionalDamageSerial =
        Snapshot.Health.FunctionalDamageSerial;
    return FPinkCabVehicleStateSnapshotCodec::Restore(
        StateSnapshot,
        OutHealth,
        OutLoad);
}

FPinkCabVehicleSnapshotCodec::ESchemaLayout
FPinkCabVehicleSnapshotCodec::ResolveSchemaLayout(
    const FPinkCabVehicleSnapshot& Snapshot)
{
    const int32 Count = Snapshot.Health.ChannelHealth.Num();
    if (Snapshot.SchemaVersion
            == FPinkCabVehicleSnapshot::LegacySchemaVersion
        && Count
            == FPinkCabVehicleSnapshot::LegacySchema1HealthChannelCount)
    {
        return ESchemaLayout::LegacyV1;
    }
    if (Snapshot.SchemaVersion
            == FPinkCabVehicleSnapshot::DivergentSchemaVersion
        && Count
            == FPinkCabVehicleSnapshot::RuntimeSchema2HealthChannelCount)
    {
        return ESchemaLayout::RuntimeV2;
    }
    if (Snapshot.SchemaVersion
            == FPinkCabVehicleSnapshot::DivergentSchemaVersion
        && Count
            == FPinkCabVehicleSnapshot::PresentationSchema2HealthChannelCount)
    {
        return ESchemaLayout::PresentationV2;
    }
    if (Snapshot.SchemaVersion
            == FPinkCabVehicleSnapshot::CurrentSchemaVersion
        && Count == static_cast<int32>(
            EPinkCabVehicleHealthChannel::Count))
    {
        return ESchemaLayout::Current;
    }
    return ESchemaLayout::Invalid;
}

void FPinkCabVehicleSnapshotCodec::InitializeHealth(
    FPinkCabVehicleHealthStateSnapshot& Health)
{
    Health.ChannelHealth.Init(
        1.0f,
        static_cast<int32>(EPinkCabVehicleHealthChannel::Count));
}

void FPinkCabVehicleSnapshotCodec::MigrateLegacyV1(
    const FPinkCabVehicleSnapshot& Snapshot,
    FPinkCabVehicleHealthStateSnapshot& Health)
{
    const TArray<float>& Saved = Snapshot.Health.ChannelHealth;
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
}

void FPinkCabVehicleSnapshotCodec::MigrateRuntimeV2(
    const FPinkCabVehicleSnapshot& Snapshot,
    FPinkCabVehicleHealthStateSnapshot& Health)
{
    const TArray<float>& Saved = Snapshot.Health.ChannelHealth;
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
}

void FPinkCabVehicleSnapshotCodec::MigratePresentationV2(
    const FPinkCabVehicleSnapshot& Snapshot,
    FPinkCabVehicleHealthStateSnapshot& Health)
{
    const TArray<float>& Saved = Snapshot.Health.ChannelHealth;
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
}

void FPinkCabVehicleSnapshotCodec::MigrateCurrent(
    const FPinkCabVehicleSnapshot& Snapshot,
    FPinkCabVehicleHealthStateSnapshot& Health)
{
    const TArray<float>& Saved = Snapshot.Health.ChannelHealth;
    for (int32 Index = 0; Index < Saved.Num(); ++Index)
    {
        Health.ChannelHealth[Index] = Saved[Index];
    }
    Health.ClutchTemperature01 = Snapshot.Health.ClutchTemperature01;
    Health.BrakeTemperature01 = Snapshot.Health.BrakeTemperature01;
}

void FPinkCabVehicleSnapshotCodec::MigrateHealth(
    const FPinkCabVehicleSnapshot& Snapshot,
    FPinkCabVehicleHealthStateSnapshot& Health)
{
    InitializeHealth(Health);
    switch (ResolveSchemaLayout(Snapshot))
    {
    case ESchemaLayout::LegacyV1:
        MigrateLegacyV1(Snapshot, Health);
        return;
    case ESchemaLayout::RuntimeV2:
        MigrateRuntimeV2(Snapshot, Health);
        return;
    case ESchemaLayout::PresentationV2:
        MigratePresentationV2(Snapshot, Health);
        return;
    case ESchemaLayout::Current:
        MigrateCurrent(Snapshot, Health);
        return;
    default:
        return;
    }
}

bool FPinkCabVehicleSnapshotCodec::ValidateItem(
    const FPinkCabVehicleLoadItemSnapshot& Item)
{
    return FMath::IsFinite(Item.MassKg)
        && FMath::IsFinite(Item.LongitudinalCm)
        && Item.MassKg > 0.0f;
}

bool FPinkCabVehicleSnapshotCodec::ValidateHealthChannels(
    const FPinkCabVehicleSnapshot& Snapshot)
{
    if (ResolveSchemaLayout(Snapshot) == ESchemaLayout::Invalid)
    {
        return false;
    }

    for (const float Value : Snapshot.Health.ChannelHealth)
    {
        if (!FMath::IsFinite(Value)
            || Value < 0.0f
            || Value > 1.0f)
        {
            return false;
        }
    }
    return true;
}

bool FPinkCabVehicleSnapshotCodec::ValidateThermalFields(
    const FPinkCabVehicleSnapshot& Snapshot,
    ESchemaLayout Layout)
{
    const bool bHasThermalFields =
        Layout == ESchemaLayout::RuntimeV2
        || Layout == ESchemaLayout::Current;
    if (!bHasThermalFields)
    {
        return true;
    }

    return FMath::IsFinite(Snapshot.Health.ClutchTemperature01)
        && FMath::IsFinite(Snapshot.Health.BrakeTemperature01)
        && Snapshot.Health.ClutchTemperature01 >= 0.0f
        && Snapshot.Health.ClutchTemperature01 <= 1.0f
        && Snapshot.Health.BrakeTemperature01 >= 0.0f
        && Snapshot.Health.BrakeTemperature01 <= 1.0f;
}

bool FPinkCabVehicleSnapshotCodec::ValidateLoadScalars(
    const FPinkCabVehicleSnapshot& Snapshot)
{
    return FMath::IsFinite(Snapshot.Load.FuelMassKg)
        && FMath::IsFinite(Snapshot.Load.FuelLongitudinalCm)
        && FMath::IsFinite(Snapshot.Load.HeroineMassKg)
        && FMath::IsFinite(Snapshot.Load.DaughterMassKg)
        && Snapshot.Load.FuelMassKg >= 0.0f
        && Snapshot.Load.HeroineMassKg >= 0.0f
        && Snapshot.Load.DaughterMassKg >= 0.0f;
}

bool FPinkCabVehicleSnapshotCodec::ValidateLoadItems(
    const FPinkCabVehicleSnapshot& Snapshot)
{
    for (const FPinkCabVehicleLoadItemSnapshot& Item
        : Snapshot.Load.Passengers)
    {
        if (!ValidateItem(Item))
        {
            return false;
        }
    }

    for (const FPinkCabVehicleLoadItemSnapshot& Item
        : Snapshot.Load.FarePassengers)
    {
        if (!ValidateItem(Item))
        {
            return false;
        }
    }
    return true;
}

bool FPinkCabVehicleSnapshotCodec::ValidateFarePassengerGroup(
    const FPinkCabVehicleSnapshot& Snapshot)
{
    if (!Snapshot.Load.bFarePassengerGroupActive)
    {
        return Snapshot.Load.FarePassengerGroupId.IsEmpty()
            && Snapshot.Load.FarePassengers.IsEmpty();
    }

    FPinkCabStableId GroupId;
    if (!FPinkCabStableId::TryParse(
            Snapshot.Load.FarePassengerGroupId,
            GroupId))
    {
        return false;
    }
    return Snapshot.Load.FarePassengers.Num() >= 1
        && Snapshot.Load.FarePassengers.Num() <= 5;
}

bool FPinkCabVehicleSnapshotCodec::Validate(
    const FPinkCabVehicleSnapshot& Snapshot)
{
    const ESchemaLayout Layout = ResolveSchemaLayout(Snapshot);
    if (Layout == ESchemaLayout::Invalid)
    {
        return false;
    }
    if (!ValidateHealthChannels(Snapshot))
    {
        return false;
    }
    if (!ValidateThermalFields(Snapshot, Layout))
    {
        return false;
    }
    if (!ValidateLoadScalars(Snapshot))
    {
        return false;
    }
    if (!ValidateLoadItems(Snapshot))
    {
        return false;
    }
    return ValidateFarePassengerGroup(Snapshot);
}
