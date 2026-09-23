#include "Persistence/PinkCabServiceSnapshotArchive.h"

void FPinkCabServiceSnapshotArchive::Serialize(
    FArchive& Ar,
    FPinkCabServiceSnapshot& Snapshot)
{
    SerializeServiceSnapshot(Ar, Snapshot);
}

void FPinkCabServiceSnapshotArchive::SerializeOwnerIdentity(
    FArchive& Ar,
    FPinkCabServiceOwnerIdentity& Owners)
{
    SerializeStableId(Ar, Owners.VehicleId);
    SerializeStableId(Ar, Owners.BuildOwnerId);
    SerializeStableId(Ar, Owners.HealthOwnerId);
    SerializeStableId(Ar, Owners.InventoryOwnerId);
    SerializeStableId(Ar, Owners.EconomyOwnerId);
}

void FPinkCabServiceSnapshotArchive::SerializeServiceContext(
    FArchive& Ar,
    FPinkCabServiceContext& Context)
{
    SerializeCityIdentity(Ar, Context.City);
    Ar << Context.SemanticKey;
    Ar << Context.ServiceNodeId;
    SerializeOwnerIdentity(Ar, Context.Owners);
}

void FPinkCabServiceSnapshotArchive::SerializeOwnedPart(
    FArchive& Ar,
    FPinkCabOwnedPartSnapshot& Part)
{
    Ar << Part.PartId;
    Ar << Part.Quantity;
}

void FPinkCabServiceSnapshotArchive::SerializeInstalledPart(
    FArchive& Ar,
    FPinkCabInstalledPartSnapshot& Part)
{
    Ar << Part.SlotId;
    Ar << Part.PartId;
}

void FPinkCabServiceSnapshotArchive::SerializeMovingFuelPolicy(
    FArchive& Ar,
    FPinkCabMovingFuelPolicyInputs& Policy)
{
    Ar << Policy.QueuePolicyId;
    Ar << Policy.SettlementPolicyId;
    Ar << Policy.InsufficientFundsPolicyId;
    Ar << Policy.TargetLongitudinalGapCm;
    Ar << Policy.GapToleranceCm;
    Ar << Policy.MaxConnectionSpeedKmh;
}

void FPinkCabServiceSnapshotArchive::SerializeInventory(
    FArchive& Ar,
    FPinkCabServiceSnapshot& Snapshot)
{
    Ar << Snapshot.Inventory.MaxItems;
    Ar << Snapshot.Inventory.MaxReplayJournalEntries;
    SerializeArray(
        Ar,
        Snapshot.Inventory.Parts,
        MaxArrayElements,
        [](FArchive& A, FPinkCabOwnedPartSnapshot& Value)
        {
            SerializeOwnedPart(A, Value);
        });
    SerializeArray(
        Ar,
        Snapshot.Inventory.AppliedOperationIds,
        MaxArrayElements,
        [](FArchive& A, FString& Value)
        {
            A << Value;
        });
}

void FPinkCabServiceSnapshotArchive::SerializeBuild(
    FArchive& Ar,
    FPinkCabServiceSnapshot& Snapshot)
{
    Ar << Snapshot.Build.SchemaVersion;
    Ar << Snapshot.Build.MaxReplayJournalEntries;
    SerializeArray(
        Ar,
        Snapshot.Build.InstalledParts,
        MaxArrayElements,
        [](FArchive& A, FPinkCabInstalledPartSnapshot& Value)
        {
            SerializeInstalledPart(A, Value);
        });
    SerializeArray(
        Ar,
        Snapshot.Build.AppliedInstallOperationIds,
        MaxArrayElements,
        [](FArchive& A, FString& Value)
        {
            A << Value;
        });
}

void FPinkCabServiceSnapshotArchive::SerializeOperations(
    FArchive& Ar,
    FPinkCabServiceSnapshot& Snapshot)
{
    Ar << Snapshot.Operations.MaxReplayJournalEntries;
    SerializeArray(
        Ar,
        Snapshot.Operations.CompletedOperationIds,
        MaxArrayElements,
        [](FArchive& A, FString& Value)
        {
            A << Value;
        });
}

void FPinkCabServiceSnapshotArchive::SerializeFuel(
    FArchive& Ar,
    FPinkCabServiceSnapshot& Snapshot)
{
    Ar << Snapshot.FuelTank.CapacityLiters;
    Ar << Snapshot.FuelTank.CurrentLiters;
    Ar << Snapshot.FuelTank.MaxReplayJournalEntries;
    SerializeArray(
        Ar,
        Snapshot.FuelTank.CreditedTransactionIds,
        MaxArrayElements,
        [](FArchive& A, FString& Value)
        {
            A << Value;
        });
}

void FPinkCabServiceSnapshotArchive::SerializeMovingFuel(
    FArchive& Ar,
    FPinkCabServiceSnapshot& Snapshot)
{
    SerializeMovingFuelPolicy(
        Ar,
        Snapshot.MovingFuel.Policy);
    SerializeEnum(Ar, Snapshot.MovingFuel.State);
    SerializeEnum(Ar, Snapshot.MovingFuel.AbortReason);
    Ar << Snapshot.MovingFuel.ServiceLaneId;
}

void FPinkCabServiceSnapshotArchive::ValidateCapacities(
    FArchive& Ar,
    const FPinkCabServiceSnapshot& Snapshot)
{
    if (!IsCapacityReasonable(Snapshot.Inventory.MaxItems)
        || !IsCapacityReasonable(
            Snapshot.Inventory.MaxReplayJournalEntries)
        || !IsCapacityReasonable(
            Snapshot.Build.MaxReplayJournalEntries)
        || !IsCapacityReasonable(
            Snapshot.Operations.MaxReplayJournalEntries)
        || !IsCapacityReasonable(
            Snapshot.FuelTank.MaxReplayJournalEntries))
    {
        Ar.SetError();
    }
}

void FPinkCabServiceSnapshotArchive::SerializeServiceSnapshot(
    FArchive& Ar,
    FPinkCabServiceSnapshot& Snapshot)
{
    Ar << Snapshot.SchemaVersion;
    SerializeServiceContext(Ar, Snapshot.Context);
    SerializeInventory(Ar, Snapshot);
    SerializeBuild(Ar, Snapshot);
    SerializeOperations(Ar, Snapshot);
    SerializeFuel(Ar, Snapshot);
    SerializeMovingFuel(Ar, Snapshot);
    ValidateCapacities(Ar, Snapshot);
}
