#include "Service/PinkCabServiceOperationRuntime.h"

FPinkCabServiceOperationRuntime::FPinkCabServiceOperationRuntime(
    int32 InMaxReplayJournalEntries)
    : MaxReplayJournalEntries(
        FMath::Max(1, InMaxReplayJournalEntries))
{
}

EPinkCabServiceOperationResult
FPinkCabServiceOperationRuntime::ChargeParking(
    FPinkCabEconomyLedger& Ledger,
    const FPinkCabStableId& OperationId,
    int64 AmountMinor)
{
    if (!OperationId.IsValid() || AmountMinor <= 0)
    {
        return EPinkCabServiceOperationResult::Invalid;
    }
    if (IsCompleted(OperationId))
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (!HasCapacity())
    {
        return EPinkCabServiceOperationResult::CapacityExceeded;
    }

    const FPinkCabEconomyTransaction Tx =
        FPinkCabEconomyTransaction::Debit(
            FPinkCabTransactionId(OperationId.Serialize()),
            EPinkCabTransactionType::Parking,
            AmountMinor,
            EPinkCabEconomyPurpose::OrdinaryPurchase);
    const EPinkCabSettlementResult Settlement =
        Ledger.Commit(Tx);
    if (Settlement == EPinkCabSettlementResult::Duplicate)
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (Settlement != EPinkCabSettlementResult::Committed)
    {
        return EPinkCabServiceOperationResult::SettlementRejected;
    }

    MarkCompleted(OperationId);
    return EPinkCabServiceOperationResult::Applied;
}

EPinkCabServiceOperationResult
FPinkCabServiceOperationRuntime::PurchasePart(
    FPinkCabEconomyLedger& Ledger,
    FPinkCabServiceInventory& Inventory,
    const FPinkCabStableId& OperationId,
    const FPinkCabPartDefinition& Definition)
{
    if (!OperationId.IsValid() || !Definition.IsValid())
    {
        return EPinkCabServiceOperationResult::Invalid;
    }
    if (IsCompleted(OperationId))
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (!HasCapacity())
    {
        return EPinkCabServiceOperationResult::CapacityExceeded;
    }
    if (!Inventory.CanAddOwnedPart())
    {
        return EPinkCabServiceOperationResult::OwnerRejected;
    }

    const FPinkCabEconomyTransaction Tx =
        FPinkCabEconomyTransaction::Debit(
            FPinkCabTransactionId(OperationId.Serialize()),
            EPinkCabTransactionType::PartPurchase,
            Definition.PriceMinor,
            EPinkCabEconomyPurpose::OrdinaryPurchase);
    const EPinkCabSettlementResult Settlement =
        Ledger.Commit(Tx);
    if (Settlement == EPinkCabSettlementResult::Duplicate)
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (Settlement != EPinkCabSettlementResult::Committed)
    {
        return EPinkCabServiceOperationResult::SettlementRejected;
    }

    const EPinkCabInventoryMutationResult Added =
        Inventory.AddOwnedPartOnce(
            OperationId,
            Definition.PartId);
    if (Added != EPinkCabInventoryMutationResult::Applied)
    {
        return EPinkCabServiceOperationResult::OwnerRejected;
    }

    MarkCompleted(OperationId);
    return EPinkCabServiceOperationResult::Applied;
}

EPinkCabServiceOperationResult
FPinkCabServiceOperationRuntime::InstallOwnedPart(
    FPinkCabServiceInventory& Inventory,
    FPinkCabVehicleBuild& Build,
    const FPinkCabStableId& OperationId,
    const FPinkCabPartDefinition& Definition,
    FName VehicleCompatibilityTag)
{
    if (!OperationId.IsValid() || !Definition.IsValid())
    {
        return EPinkCabServiceOperationResult::Invalid;
    }
    if (IsCompleted(OperationId))
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (!HasCapacity())
    {
        return EPinkCabServiceOperationResult::CapacityExceeded;
    }
    if (Inventory.GetQuantity(Definition.PartId) <= 0)
    {
        return EPinkCabServiceOperationResult::OwnerRejected;
    }
    if (VehicleCompatibilityTag.IsNone()
        || VehicleCompatibilityTag
            != Definition.CompatibilityTag)
    {
        return EPinkCabServiceOperationResult::OwnerRejected;
    }

    const EPinkCabPartInstallResult Installed =
        Build.TryInstallPartOnce(
            OperationId,
            Definition,
            VehicleCompatibilityTag);
    if (Installed == EPinkCabPartInstallResult::Duplicate)
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (Installed != EPinkCabPartInstallResult::Applied)
    {
        return EPinkCabServiceOperationResult::OwnerRejected;
    }

    const EPinkCabInventoryMutationResult Consumed =
        Inventory.ConsumeOwnedPartOnce(
            OperationId,
            Definition.PartId);
    if (Consumed != EPinkCabInventoryMutationResult::Applied)
    {
        return EPinkCabServiceOperationResult::OwnerRejected;
    }

    MarkCompleted(OperationId);
    return EPinkCabServiceOperationResult::Applied;
}

EPinkCabServiceOperationResult
FPinkCabServiceOperationRuntime::Repair(
    FPinkCabEconomyLedger& Ledger,
    FPinkCabVehicleHealthState& Health,
    const FPinkCabStableId& OperationId,
    int64 AmountMinor,
    EPinkCabEconomyPurpose Purpose,
    const TArray<FPinkCabRepairLine>& Lines)
{
    if (!OperationId.IsValid())
    {
        return EPinkCabServiceOperationResult::Invalid;
    }
    if (IsCompleted(OperationId))
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (!HasCapacity())
    {
        return EPinkCabServiceOperationResult::CapacityExceeded;
    }

    FPinkCabRepairRequest Request;
    if (!FPinkCabRepairService::BuildRequest(
            Health,
            FPinkCabTransactionId(OperationId.Serialize()),
            AmountMinor,
            Purpose,
            Lines,
            Request))
    {
        return EPinkCabServiceOperationResult::Invalid;
    }

    const EPinkCabSettlementResult Settlement =
        Ledger.Commit(Request.Transaction);
    if (Settlement == EPinkCabSettlementResult::Duplicate)
    {
        return EPinkCabServiceOperationResult::Duplicate;
    }
    if (Settlement != EPinkCabSettlementResult::Committed)
    {
        return EPinkCabServiceOperationResult::SettlementRejected;
    }
    if (!FPinkCabRepairService::ApplyCommittedRepair(
            Health,
            Request))
    {
        return EPinkCabServiceOperationResult::OwnerRejected;
    }

    MarkCompleted(OperationId);
    return EPinkCabServiceOperationResult::Applied;
}

bool FPinkCabServiceOperationRuntime::IsCompleted(
    const FPinkCabStableId& OperationId) const
{
    return OperationId.IsValid()
        && CompletedOperationIds.Contains(
            OperationId.Serialize());
}

int32 FPinkCabServiceOperationRuntime::GetCompletedOperationCount() const
{
    return CompletedOperationIds.Num();
}

int32 FPinkCabServiceOperationRuntime::GetMaxReplayJournalEntries() const
{
    return MaxReplayJournalEntries;
}

bool FPinkCabServiceOperationRuntime::HasCapacity() const
{
    return CompletedOperationIds.Num()
        < MaxReplayJournalEntries;
}

void FPinkCabServiceOperationRuntime::MarkCompleted(
    const FPinkCabStableId& OperationId)
{
    CompletedOperationIds.Add(OperationId.Serialize());
}
