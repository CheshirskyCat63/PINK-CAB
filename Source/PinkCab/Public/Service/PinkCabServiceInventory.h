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

class PINKCAB_API FPinkCabServiceInventory
{
public:
    explicit FPinkCabServiceInventory(
        int32 InMaxItems = 64,
        int32 InMaxReplayJournalEntries = 4096);

    EPinkCabInventoryMutationResult AddOwnedPartOnce(
        const FPinkCabStableId& OperationId,
        const FString& StablePartId);
    EPinkCabInventoryMutationResult ConsumeOwnedPartOnce(
        const FPinkCabStableId& OperationId,
        const FString& StablePartId);
    int32 GetQuantity(const FString& StablePartId) const;
    int32 GetTotalQuantity() const;
    bool CanAddOwnedPart() const;
    int32 GetMaxItems() const;
    int32 GetMaxReplayJournalEntries() const;
    int32 GetOperationCount() const;

private:
    friend class FPinkCabServiceSnapshotCodec;

    int32 MaxItems = 64;
    int32 MaxReplayJournalEntries = 4096;
    TMap<FString, int32> Quantities;
    TSet<FString> AppliedOperationIds;
};
