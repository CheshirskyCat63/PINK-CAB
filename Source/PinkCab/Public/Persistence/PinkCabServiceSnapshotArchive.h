#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class FPinkCabServiceSnapshotArchive : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(FArchive& Ar, FPinkCabServiceSnapshot& Snapshot)
    {
        SerializeServiceSnapshot(Ar, Snapshot);
    }

private:
    static void SerializeOwnerIdentity(FArchive& Ar, FPinkCabServiceOwnerIdentity& Owners)
    {
        SerializeStableId(Ar, Owners.VehicleId);
        SerializeStableId(Ar, Owners.BuildOwnerId);
        SerializeStableId(Ar, Owners.HealthOwnerId);
        SerializeStableId(Ar, Owners.InventoryOwnerId);
        SerializeStableId(Ar, Owners.EconomyOwnerId);
    }

    static void SerializeServiceContext(FArchive& Ar, FPinkCabServiceContext& Context)
    {
        SerializeCityIdentity(Ar, Context.City);
        Ar << Context.SemanticKey;
        Ar << Context.ServiceNodeId;
        SerializeOwnerIdentity(Ar, Context.Owners);
    }

    static void SerializeOwnedPart(FArchive& Ar, FPinkCabOwnedPartSnapshot& Part)
    {
        Ar << Part.PartId;
        Ar << Part.Quantity;
    }

    static void SerializeInstalledPart(FArchive& Ar, FPinkCabInstalledPartSnapshot& Part)
    {
        Ar << Part.SlotId;
        Ar << Part.PartId;
    }
    static void SerializeMovingFuelPolicy(FArchive& Ar, FPinkCabMovingFuelPolicyInputs& Policy)
    {
        Ar << Policy.QueuePolicyId;
        Ar << Policy.SettlementPolicyId;
        Ar << Policy.InsufficientFundsPolicyId;
        Ar << Policy.TargetLongitudinalGapCm;
        Ar << Policy.GapToleranceCm;
        Ar << Policy.MaxConnectionSpeedKmh;
    }

    static void SerializeServiceSnapshot(FArchive& Ar, FPinkCabServiceSnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        SerializeServiceContext(Ar, Snapshot.Context);
        Ar << Snapshot.Inventory.MaxItems;
        Ar << Snapshot.Inventory.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.Inventory.Parts, MaxArrayElements,
            [](FArchive& A, FPinkCabOwnedPartSnapshot& Value) { SerializeOwnedPart(A, Value); });
        SerializeArray(Ar, Snapshot.Inventory.AppliedOperationIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        Ar << Snapshot.Build.SchemaVersion;
        Ar << Snapshot.Build.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.Build.InstalledParts, MaxArrayElements,
            [](FArchive& A, FPinkCabInstalledPartSnapshot& Value) { SerializeInstalledPart(A, Value); });
        SerializeArray(Ar, Snapshot.Build.AppliedInstallOperationIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        Ar << Snapshot.Operations.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.Operations.CompletedOperationIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        Ar << Snapshot.FuelTank.CapacityLiters;
        Ar << Snapshot.FuelTank.CurrentLiters;
        Ar << Snapshot.FuelTank.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.FuelTank.CreditedTransactionIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        SerializeMovingFuelPolicy(Ar, Snapshot.MovingFuel.Policy);
        SerializeEnum(Ar, Snapshot.MovingFuel.State);
        SerializeEnum(Ar, Snapshot.MovingFuel.AbortReason);
        Ar << Snapshot.MovingFuel.ServiceLaneId;

        if (!IsCapacityReasonable(Snapshot.Inventory.MaxItems)
            || !IsCapacityReasonable(Snapshot.Inventory.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Snapshot.Build.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Snapshot.Operations.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Snapshot.FuelTank.MaxReplayJournalEntries))
        {
            Ar.SetError();
        }
    }
};
