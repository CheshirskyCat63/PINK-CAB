#pragma once

#include "CoreMinimal.h"
#include "Service/PinkCabServiceContext.h"
#include "Service/PinkCabServiceInventory.h"
#include "Service/PinkCabVehicleBuild.h"
#include "Service/PinkCabServiceOperationRuntime.h"
#include "Service/PinkCabFuelTank.h"
#include "Service/PinkCabMovingFuelSession.h"

struct FPinkCabOwnedPartSnapshot
{
    FString PartId;
    int32 Quantity = 0;
};

struct FPinkCabInstalledPartSnapshot
{
    FName SlotId;
    FString PartId;
};

struct FPinkCabInventorySnapshot
{
    int32 MaxItems = 0;
    int32 MaxReplayJournalEntries = 0;
    TArray<FPinkCabOwnedPartSnapshot> Parts;
    TArray<FString> AppliedOperationIds;
};
struct FPinkCabBuildSnapshot
{
    int32 SchemaVersion = 1;
    int32 MaxReplayJournalEntries = 0;
    TArray<FPinkCabInstalledPartSnapshot> InstalledParts;
    TArray<FString> AppliedInstallOperationIds;
};

struct FPinkCabServiceOperationSnapshot
{
    int32 MaxReplayJournalEntries = 0;
    TArray<FString> CompletedOperationIds;
};

struct FPinkCabFuelTankSnapshot
{
    float CapacityLiters = 0.0f;
    float CurrentLiters = 0.0f;
    int32 MaxReplayJournalEntries = 0;
    TArray<FString> CreditedTransactionIds;
};

struct FPinkCabMovingFuelSnapshot
{
    FPinkCabMovingFuelPolicyInputs Policy;
    EPinkCabMovingFuelState State = EPinkCabMovingFuelState::Disconnected;
    EPinkCabMovingFuelAbortReason AbortReason = EPinkCabMovingFuelAbortReason::None;
    FString ServiceLaneId;
};
struct FPinkCabServiceSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;

    int32 SchemaVersion = CurrentSchemaVersion;
    FPinkCabServiceContext Context;
    FPinkCabInventorySnapshot Inventory;
    FPinkCabBuildSnapshot Build;
    FPinkCabServiceOperationSnapshot Operations;
    FPinkCabFuelTankSnapshot FuelTank;
    FPinkCabMovingFuelSnapshot MovingFuel;
};

class FPinkCabServiceSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabServiceContext& Context,
        const FPinkCabServiceInventory& Inventory,
        const FPinkCabVehicleBuild& Build,
        const FPinkCabServiceOperationRuntime& Operations,
        const FPinkCabFuelTank& Tank,
        const FPinkCabMovingFuelSession& MovingFuel,
        FPinkCabServiceSnapshot& OutSnapshot)
    {
        if (!Context.IsValid()) return false;
        FPinkCabServiceSnapshot Snapshot;
        Snapshot.Context = Context;
        Snapshot.Inventory.MaxItems = Inventory.MaxItems;
        Snapshot.Inventory.MaxReplayJournalEntries = Inventory.MaxReplayJournalEntries;
        for (const TPair<FString, int32>& Pair : Inventory.Quantities)
            Snapshot.Inventory.Parts.Add({Pair.Key, Pair.Value});
        Snapshot.Inventory.Parts.Sort([](const auto& A, const auto& B) { return A.PartId < B.PartId; });
        for (const FString& Id : Inventory.AppliedOperationIds)
            Snapshot.Inventory.AppliedOperationIds.Add(Id);
        Snapshot.Inventory.AppliedOperationIds.Sort();

        Snapshot.Build.SchemaVersion = Build.SchemaVersion;
        Snapshot.Build.MaxReplayJournalEntries = Build.MaxReplayJournalEntries;
        for (const TPair<FName, FString>& Pair : Build.InstalledParts)
            Snapshot.Build.InstalledParts.Add({Pair.Key, Pair.Value});
        Snapshot.Build.InstalledParts.Sort([](const auto& A, const auto& B)
            { return A.SlotId.LexicalLess(B.SlotId); });
        for (const FString& Id : Build.AppliedInstallOperationIds)
            Snapshot.Build.AppliedInstallOperationIds.Add(Id);
        Snapshot.Build.AppliedInstallOperationIds.Sort();

        Snapshot.Operations.MaxReplayJournalEntries = Operations.MaxReplayJournalEntries;
        for (const FString& Id : Operations.CompletedOperationIds)
            Snapshot.Operations.CompletedOperationIds.Add(Id);
        Snapshot.Operations.CompletedOperationIds.Sort();
        Snapshot.FuelTank.CapacityLiters = Tank.CapacityLiters;
        Snapshot.FuelTank.CurrentLiters = Tank.CurrentLiters;
        Snapshot.FuelTank.MaxReplayJournalEntries = Tank.MaxReplayJournalEntries;
        for (const FString& Id : Tank.CreditedTransactionIds)
            Snapshot.FuelTank.CreditedTransactionIds.Add(Id);
        Snapshot.FuelTank.CreditedTransactionIds.Sort();

        Snapshot.MovingFuel.Policy = MovingFuel.Policy;
        Snapshot.MovingFuel.State = MovingFuel.State;
        Snapshot.MovingFuel.AbortReason = MovingFuel.AbortReason;
        Snapshot.MovingFuel.ServiceLaneId = MovingFuel.ServiceLaneId.Serialize();

        if (!Validate(Snapshot)) return false;
        OutSnapshot = MoveTemp(Snapshot);
        return true;
    }

    static bool Restore(
        const FPinkCabServiceSnapshot& Snapshot,
        FPinkCabServiceContext& OutContext,
        FPinkCabServiceInventory& OutInventory,
        FPinkCabVehicleBuild& OutBuild,
        FPinkCabServiceOperationRuntime& OutOperations,
        FPinkCabFuelTank& OutTank,
        FPinkCabMovingFuelSession& OutMovingFuel)
    {
        if (!Validate(Snapshot)) return false;
        FPinkCabServiceInventory Inventory(
            Snapshot.Inventory.MaxItems,
            Snapshot.Inventory.MaxReplayJournalEntries);
        for (const FPinkCabOwnedPartSnapshot& Part : Snapshot.Inventory.Parts)
        {
            Inventory.Quantities.Add(Part.PartId, Part.Quantity);
        }
        for (const FString& Id : Snapshot.Inventory.AppliedOperationIds)
        {
            Inventory.AppliedOperationIds.Add(Id);
        }

        FPinkCabVehicleBuild Build(Snapshot.Build.MaxReplayJournalEntries);
        Build.SchemaVersion = Snapshot.Build.SchemaVersion;
        for (const FPinkCabInstalledPartSnapshot& Part : Snapshot.Build.InstalledParts)
        {
            Build.InstalledParts.Add(Part.SlotId, Part.PartId);
        }
        for (const FString& Id : Snapshot.Build.AppliedInstallOperationIds)
        {
            Build.AppliedInstallOperationIds.Add(Id);
        }

        FPinkCabServiceOperationRuntime Operations(
            Snapshot.Operations.MaxReplayJournalEntries);
        for (const FString& Id : Snapshot.Operations.CompletedOperationIds)
        {
            Operations.CompletedOperationIds.Add(Id);
        }
        FPinkCabFuelTank Tank(
            Snapshot.FuelTank.CapacityLiters,
            Snapshot.FuelTank.CurrentLiters,
            Snapshot.FuelTank.MaxReplayJournalEntries);
        for (const FString& Id : Snapshot.FuelTank.CreditedTransactionIds)
        {
            Tank.CreditedTransactionIds.Add(Id);
        }

        FPinkCabMovingFuelSession MovingFuel(Snapshot.MovingFuel.Policy);
        MovingFuel.State = Snapshot.MovingFuel.State;
        MovingFuel.AbortReason = Snapshot.MovingFuel.AbortReason;
        MovingFuel.ServiceLaneId = FPinkCabLaneId(Snapshot.MovingFuel.ServiceLaneId);

        OutContext = Snapshot.Context;
        OutInventory = MoveTemp(Inventory);
        OutBuild = MoveTemp(Build);
        OutOperations = MoveTemp(Operations);
        OutTank = MoveTemp(Tank);
        OutMovingFuel = MoveTemp(MovingFuel);
        return true;
    }
private:
    static bool ValidateIds(const TArray<FString>& Ids, int32 MaxEntries)
    {
        if (MaxEntries <= 0 || Ids.Num() > MaxEntries) return false;
        TSet<FString> Unique;
        for (const FString& Id : Ids)
        {
            const FString Clean = Id.TrimStartAndEnd();
            if (Clean.IsEmpty() || Unique.Contains(Clean)) return false;
            Unique.Add(Clean);
        }
        return true;
    }
    static bool Validate(const FPinkCabServiceSnapshot& Snapshot)
    {
        if (Snapshot.SchemaVersion != FPinkCabServiceSnapshot::CurrentSchemaVersion)
            return false;
        if (!Snapshot.Context.IsValid())
            return false;
        if (Snapshot.Inventory.MaxItems <= 0)
            return false;
        if (!ValidateIds(
            Snapshot.Inventory.AppliedOperationIds,
            Snapshot.Inventory.MaxReplayJournalEntries))
            return false;

        int32 TotalItems = 0;
        TSet<FString> PartIds;
        for (const FPinkCabOwnedPartSnapshot& Part : Snapshot.Inventory.Parts)
        {
            if (Part.PartId.TrimStartAndEnd().IsEmpty() || Part.Quantity <= 0)
                return false;
            if (PartIds.Contains(Part.PartId)) return false;
            PartIds.Add(Part.PartId);
            TotalItems += Part.Quantity;
        }
        if (TotalItems > Snapshot.Inventory.MaxItems) return false;
        if (Snapshot.Build.SchemaVersion != 1)
            return false;
        if (!ValidateIds(
            Snapshot.Build.AppliedInstallOperationIds,
            Snapshot.Build.MaxReplayJournalEntries))
            return false;
        TSet<FName> Slots;
        for (const FPinkCabInstalledPartSnapshot& Part : Snapshot.Build.InstalledParts)
        {
            if (Part.SlotId.IsNone() || Part.PartId.TrimStartAndEnd().IsEmpty())
                return false;
            if (Slots.Contains(Part.SlotId)) return false;
            Slots.Add(Part.SlotId);
        }

        if (!ValidateIds(
            Snapshot.Operations.CompletedOperationIds,
            Snapshot.Operations.MaxReplayJournalEntries))
            return false;
        if (!FMath::IsFinite(Snapshot.FuelTank.CapacityLiters)
            || !FMath::IsFinite(Snapshot.FuelTank.CurrentLiters)
            || Snapshot.FuelTank.CapacityLiters < 0.0f
            || Snapshot.FuelTank.CurrentLiters < 0.0f
            || Snapshot.FuelTank.CurrentLiters > Snapshot.FuelTank.CapacityLiters)
            return false;
        if (!ValidateIds(
            Snapshot.FuelTank.CreditedTransactionIds,
            Snapshot.FuelTank.MaxReplayJournalEntries))
            return false;

        if (!Snapshot.MovingFuel.Policy.IsSpecified())
            return false;
        const uint8 StateValue = static_cast<uint8>(Snapshot.MovingFuel.State);
        const uint8 AbortValue = static_cast<uint8>(Snapshot.MovingFuel.AbortReason);
        if (StateValue > static_cast<uint8>(EPinkCabMovingFuelState::Aborted)
            || AbortValue > static_cast<uint8>(EPinkCabMovingFuelAbortReason::InvalidLane))
            return false;
        if ((Snapshot.MovingFuel.State == EPinkCabMovingFuelState::Connected
                || Snapshot.MovingFuel.State == EPinkCabMovingFuelState::Fueling)
            && Snapshot.MovingFuel.ServiceLaneId.TrimStartAndEnd().IsEmpty())
            return false;
        if (Snapshot.MovingFuel.State == EPinkCabMovingFuelState::Aborted
            && Snapshot.MovingFuel.AbortReason == EPinkCabMovingFuelAbortReason::None)
            return false;
        return true;
    }
};
