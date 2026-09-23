#include "Persistence/PinkCabServiceSnapshotCodec.h"

bool FPinkCabServiceSnapshotCodec::Capture(
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

bool FPinkCabServiceSnapshotCodec::Restore(
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
bool FPinkCabServiceSnapshotCodec::ValidateIds(const TArray<FString>& Ids, int32 MaxEntries)
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
bool FPinkCabServiceSnapshotCodec::ValidateInventory(
    const FPinkCabInventorySnapshot& Inventory)
{
    if (Inventory.MaxItems <= 0
        || !ValidateIds(Inventory.AppliedOperationIds, Inventory.MaxReplayJournalEntries))
    {
        return false;
    }
    int32 TotalItems = 0;
    TSet<FString> PartIds;
    for (const FPinkCabOwnedPartSnapshot& Part : Inventory.Parts)
    {
        if (Part.PartId.TrimStartAndEnd().IsEmpty()
            || Part.Quantity <= 0
            || PartIds.Contains(Part.PartId))
        {
            return false;
        }
        PartIds.Add(Part.PartId);
        TotalItems += Part.Quantity;
    }
    return TotalItems <= Inventory.MaxItems;
}

bool FPinkCabServiceSnapshotCodec::ValidateBuild(const FPinkCabBuildSnapshot& Build)
{
    if (Build.SchemaVersion != 1
        || !ValidateIds(Build.AppliedInstallOperationIds, Build.MaxReplayJournalEntries))
    {
        return false;
    }
    TSet<FName> Slots;
    for (const FPinkCabInstalledPartSnapshot& Part : Build.InstalledParts)
    {
        if (Part.SlotId.IsNone()
            || Part.PartId.TrimStartAndEnd().IsEmpty()
            || Slots.Contains(Part.SlotId))
        {
            return false;
        }
        Slots.Add(Part.SlotId);
    }
    return true;
}

bool FPinkCabServiceSnapshotCodec::ValidateFuelTank(const FPinkCabFuelTankSnapshot& FuelTank)
{
    return FMath::IsFinite(FuelTank.CapacityLiters)
        && FMath::IsFinite(FuelTank.CurrentLiters)
        && FuelTank.CapacityLiters >= 0.0f
        && FuelTank.CurrentLiters >= 0.0f
        && FuelTank.CurrentLiters <= FuelTank.CapacityLiters
        && ValidateIds(FuelTank.CreditedTransactionIds, FuelTank.MaxReplayJournalEntries);
}

bool FPinkCabServiceSnapshotCodec::ValidateMovingFuel(
    const FPinkCabMovingFuelSnapshot& MovingFuel)
{
    if (!MovingFuel.Policy.IsSpecified()) return false;
    const uint8 StateValue = static_cast<uint8>(MovingFuel.State);
    const uint8 AbortValue = static_cast<uint8>(MovingFuel.AbortReason);
    if (StateValue > static_cast<uint8>(EPinkCabMovingFuelState::Aborted)
        || AbortValue > static_cast<uint8>(EPinkCabMovingFuelAbortReason::InvalidLane))
    {
        return false;
    }
    const bool bNeedsLane = MovingFuel.State == EPinkCabMovingFuelState::Connected
        || MovingFuel.State == EPinkCabMovingFuelState::Fueling;
    if (bNeedsLane && MovingFuel.ServiceLaneId.TrimStartAndEnd().IsEmpty()) return false;
    return MovingFuel.State != EPinkCabMovingFuelState::Aborted
        || MovingFuel.AbortReason != EPinkCabMovingFuelAbortReason::None;
}

bool FPinkCabServiceSnapshotCodec::Validate(const FPinkCabServiceSnapshot& Snapshot)
{
    return Snapshot.SchemaVersion == FPinkCabServiceSnapshot::CurrentSchemaVersion
        && Snapshot.Context.IsValid()
        && ValidateInventory(Snapshot.Inventory)
        && ValidateBuild(Snapshot.Build)
        && ValidateIds(
            Snapshot.Operations.CompletedOperationIds,
            Snapshot.Operations.MaxReplayJournalEntries)
        && ValidateFuelTank(Snapshot.FuelTank)
        && ValidateMovingFuel(Snapshot.MovingFuel);
}
