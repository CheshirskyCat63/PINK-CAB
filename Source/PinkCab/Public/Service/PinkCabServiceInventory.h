#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

class FPinkCabServiceSnapshotCodec;

enum class EPinkCabInventoryMutationResult : uint8
{
    Applied,
    Duplicate,
    Invalid,
    CapacityExceeded,
    NotOwned
};

class FPinkCabServiceInventory
{
public:
    explicit FPinkCabServiceInventory(
        int32 InMaxItems = 64,
        int32 InMaxReplayJournalEntries = 4096)
        : MaxItems(FMath::Max(1, InMaxItems))
        , MaxReplayJournalEntries(FMath::Max(1, InMaxReplayJournalEntries))
    {
    }
    EPinkCabInventoryMutationResult AddOwnedPartOnce(
        const FPinkCabStableId& OperationId,
        const FString& StablePartId)
    {
        const FString PartId = StablePartId.TrimStartAndEnd();
        if (!OperationId.IsValid() || PartId.IsEmpty()) return EPinkCabInventoryMutationResult::Invalid;
        const FString OpKey = OperationId.Serialize();
        if (AppliedOperationIds.Contains(OpKey)) return EPinkCabInventoryMutationResult::Duplicate;
        if (AppliedOperationIds.Num() >= MaxReplayJournalEntries
            || GetTotalQuantity() >= MaxItems)
            return EPinkCabInventoryMutationResult::CapacityExceeded;
        ++Quantities.FindOrAdd(PartId);
        AppliedOperationIds.Add(OpKey);
        return EPinkCabInventoryMutationResult::Applied;
    }

    EPinkCabInventoryMutationResult ConsumeOwnedPartOnce(
        const FPinkCabStableId& OperationId,
        const FString& StablePartId)
    {
        const FString PartId = StablePartId.TrimStartAndEnd();
        if (!OperationId.IsValid() || PartId.IsEmpty()) return EPinkCabInventoryMutationResult::Invalid;
        const FString OpKey = OperationId.Serialize();
        if (AppliedOperationIds.Contains(OpKey)) return EPinkCabInventoryMutationResult::Duplicate;
        int32* Quantity = Quantities.Find(PartId);
        if (!Quantity || *Quantity <= 0) return EPinkCabInventoryMutationResult::NotOwned;
        if (AppliedOperationIds.Num() >= MaxReplayJournalEntries)
            return EPinkCabInventoryMutationResult::CapacityExceeded;
        --(*Quantity);
        if (*Quantity == 0) Quantities.Remove(PartId);
        AppliedOperationIds.Add(OpKey);
        return EPinkCabInventoryMutationResult::Applied;
    }

    int32 GetQuantity(const FString& StablePartId) const
    {
        const int32* Found = Quantities.Find(StablePartId.TrimStartAndEnd());
        return Found ? *Found : 0;
    }

    int32 GetTotalQuantity() const
    {
        int32 Total = 0;
        for (const TPair<FString, int32>& Pair : Quantities) Total += Pair.Value;
        return Total;
    }

    bool CanAddOwnedPart() const
    {
        return GetTotalQuantity() < MaxItems
            && AppliedOperationIds.Num() < MaxReplayJournalEntries;
    }
    int32 GetMaxItems() const { return MaxItems; }
    int32 GetMaxReplayJournalEntries() const { return MaxReplayJournalEntries; }
    int32 GetOperationCount() const { return AppliedOperationIds.Num(); }

private:
    friend class FPinkCabServiceSnapshotCodec;

    int32 MaxItems = 64;
    int32 MaxReplayJournalEntries = 4096;
    TMap<FString, int32> Quantities;
    TSet<FString> AppliedOperationIds;
};
