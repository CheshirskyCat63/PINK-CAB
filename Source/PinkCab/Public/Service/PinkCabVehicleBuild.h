#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Service/PinkCabPartCatalog.h"

class FPinkCabServiceSnapshotCodec;

enum class EPinkCabPartInstallResult : uint8
{
    Applied,
    Duplicate,
    InvalidDefinition,
    Incompatible,
    CapacityExceeded
};

class PINKCAB_API FPinkCabVehicleBuild
{
public:
    explicit FPinkCabVehicleBuild(
        int32 InMaxReplayJournalEntries = 4096);

    int32 GetSchemaVersion() const;
    bool InstallPart(
        FName SlotId,
        const FString& StablePartId);
    EPinkCabPartInstallResult TryInstallPartOnce(
        const FPinkCabStableId& OperationId,
        const FPinkCabPartDefinition& Definition,
        FName VehicleCompatibilityTag);
    FString GetInstalledPartId(FName SlotId) const;
    int32 GetAppliedInstallCount() const;
    int32 GetMaxReplayJournalEntries() const;

private:
    friend class FPinkCabServiceSnapshotCodec;

    int32 SchemaVersion = 1;
    int32 MaxReplayJournalEntries = 4096;
    TMap<FName, FString> InstalledParts;
    TSet<FString> AppliedInstallOperationIds;
};
