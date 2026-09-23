#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Service/PinkCabPartCatalog.h"
#include "Service/PinkCabRepairService.h"
#include "Service/PinkCabServiceInventory.h"
#include "Service/PinkCabVehicleBuild.h"

class FPinkCabServiceSnapshotCodec;

enum class EPinkCabServiceOperationResult : uint8
{
    Applied,
    Duplicate,
    Invalid,
    SettlementRejected,
    OwnerRejected,
    CapacityExceeded
};

class PINKCAB_API FPinkCabServiceOperationRuntime
{
public:
    explicit FPinkCabServiceOperationRuntime(
        int32 InMaxReplayJournalEntries = 4096);

    EPinkCabServiceOperationResult ChargeParking(
        FPinkCabEconomyLedger& Ledger,
        const FPinkCabStableId& OperationId,
        int64 AmountMinor);

    EPinkCabServiceOperationResult PurchasePart(
        FPinkCabEconomyLedger& Ledger,
        FPinkCabServiceInventory& Inventory,
        const FPinkCabStableId& OperationId,
        const FPinkCabPartDefinition& Definition);

    EPinkCabServiceOperationResult InstallOwnedPart(
        FPinkCabServiceInventory& Inventory,
        FPinkCabVehicleBuild& Build,
        const FPinkCabStableId& OperationId,
        const FPinkCabPartDefinition& Definition,
        FName VehicleCompatibilityTag);

    EPinkCabServiceOperationResult Repair(
        FPinkCabEconomyLedger& Ledger,
        FPinkCabVehicleHealthState& Health,
        const FPinkCabStableId& OperationId,
        int64 AmountMinor,
        EPinkCabEconomyPurpose Purpose,
        const TArray<FPinkCabRepairLine>& Lines);

    bool IsCompleted(
        const FPinkCabStableId& OperationId) const;
    int32 GetCompletedOperationCount() const;
    int32 GetMaxReplayJournalEntries() const;

private:
    friend class FPinkCabServiceSnapshotCodec;

    bool HasCapacity() const;
    void MarkCompleted(
        const FPinkCabStableId& OperationId);

    int32 MaxReplayJournalEntries = 4096;
    TSet<FString> CompletedOperationIds;
};
