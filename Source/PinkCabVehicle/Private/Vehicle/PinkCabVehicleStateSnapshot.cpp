#include "Vehicle/PinkCabVehicleStateSnapshot.h"

bool FPinkCabVehicleStateSnapshotCodec::Capture(
    const FPinkCabVehicleHealthState& Health,
    const FPinkCabVehicleLoadState& Load,
    FPinkCabVehicleStateSnapshot& OutSnapshot)
{
    FPinkCabVehicleStateSnapshot Snapshot;
    for (int32 Index = 0;
         Index < static_cast<int32>(EPinkCabVehicleHealthChannel::Count);
         ++Index)
    {
        Snapshot.Health.ChannelHealth.Add(Health.Health[Index]);
    }
    Snapshot.Health.FunctionalDamageSerial = Health.FunctionalDamageSerial;
    Snapshot.Health.ClutchTemperature01 = Health.ClutchTemperature01;
    Snapshot.Health.BrakeTemperature01 = Health.BrakeTemperature01;
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
    if (!Validate(Snapshot)) return false;
    OutSnapshot = MoveTemp(Snapshot);
    return true;
}

bool FPinkCabVehicleStateSnapshotCodec::Restore(
    const FPinkCabVehicleStateSnapshot& Snapshot,
    FPinkCabVehicleHealthState& OutHealth,
    FPinkCabVehicleLoadState& OutLoad)
{
    if (!Validate(Snapshot)) return false;
    FPinkCabVehicleHealthState Health;
    for (int32 Index = 0; Index < Snapshot.Health.ChannelHealth.Num(); ++Index)
    {
        Health.Health[Index] = Snapshot.Health.ChannelHealth[Index];
    }
    Health.FunctionalDamageSerial = Snapshot.Health.FunctionalDamageSerial;
    Health.ClutchTemperature01 = Snapshot.Health.ClutchTemperature01;
    Health.BrakeTemperature01 = Snapshot.Health.BrakeTemperature01;

    FPinkCabVehicleLoadState Load;
    Load.FuelMassKg = Snapshot.Load.FuelMassKg;
    Load.FuelLongitudinalCm = Snapshot.Load.FuelLongitudinalCm;
    Load.HeroineMassKg = Snapshot.Load.HeroineMassKg;
    Load.DaughterMassKg = Snapshot.Load.DaughterMassKg;
    for (const FPinkCabVehicleLoadItemStateSnapshot& Saved : Snapshot.Load.Passengers)
    {
        Load.Passengers.Add(FPinkCabVehicleLoadItem(Saved.MassKg, Saved.LongitudinalCm));
    }
    for (const FPinkCabVehicleLoadItemStateSnapshot& Saved : Snapshot.Load.FarePassengers)
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

bool FPinkCabVehicleStateSnapshotCodec::ValidateHealth(
    const FPinkCabVehicleHealthStateSnapshot& Health)
{
    if (Health.ChannelHealth.Num() != static_cast<int32>(EPinkCabVehicleHealthChannel::Count))
    {
        return false;
    }
    for (const float Value : Health.ChannelHealth)
    {
        if (!FMath::IsFinite(Value) || Value < 0.0f || Value > 1.0f) return false;
    }
    return FMath::IsFinite(Health.ClutchTemperature01)
        && FMath::IsFinite(Health.BrakeTemperature01)
        && Health.ClutchTemperature01 >= 0.0f
        && Health.ClutchTemperature01 <= 1.0f
        && Health.BrakeTemperature01 >= 0.0f
        && Health.BrakeTemperature01 <= 1.0f;
}

bool FPinkCabVehicleStateSnapshotCodec::ValidateLoadScalars(
    const FPinkCabVehicleLoadStateSnapshot& Load)
{
    return FMath::IsFinite(Load.FuelMassKg)
        && FMath::IsFinite(Load.FuelLongitudinalCm)
        && FMath::IsFinite(Load.HeroineMassKg)
        && FMath::IsFinite(Load.DaughterMassKg)
        && Load.FuelMassKg >= 0.0f
        && Load.HeroineMassKg >= 0.0f
        && Load.DaughterMassKg >= 0.0f;
}

bool FPinkCabVehicleStateSnapshotCodec::ValidateLoadItems(
    const FPinkCabVehicleLoadStateSnapshot& Load)
{
    for (const FPinkCabVehicleLoadItemStateSnapshot& Item : Load.Passengers)
    {
        if (!ValidateItem(Item)) return false;
    }
    for (const FPinkCabVehicleLoadItemStateSnapshot& Item : Load.FarePassengers)
    {
        if (!ValidateItem(Item)) return false;
    }
    return true;
}

bool FPinkCabVehicleStateSnapshotCodec::ValidateFareGroup(
    const FPinkCabVehicleLoadStateSnapshot& Load)
{
    if (!Load.bFarePassengerGroupActive)
    {
        return Load.FarePassengerGroupId.IsEmpty() && Load.FarePassengers.IsEmpty();
    }
    FPinkCabStableId GroupId;
    return FPinkCabStableId::TryParse(Load.FarePassengerGroupId, GroupId)
        && Load.FarePassengers.Num() >= 1
        && Load.FarePassengers.Num() <= 5;
}

bool FPinkCabVehicleStateSnapshotCodec::ValidateLoad(
    const FPinkCabVehicleLoadStateSnapshot& Load)
{
    return ValidateLoadScalars(Load)
        && ValidateLoadItems(Load)
        && ValidateFareGroup(Load);
}

bool FPinkCabVehicleStateSnapshotCodec::Validate(const FPinkCabVehicleStateSnapshot& Snapshot)
{
    return ValidateHealth(Snapshot.Health) && ValidateLoad(Snapshot.Load);
}

bool FPinkCabVehicleStateSnapshotCodec::ValidateItem(const FPinkCabVehicleLoadItemStateSnapshot& Item)
{
    return FMath::IsFinite(Item.MassKg)
        && FMath::IsFinite(Item.LongitudinalCm)
        && Item.MassKg > 0.0f;
}
