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

class FPinkCabVehicleBuild
{
public:
    explicit FPinkCabVehicleBuild(int32 InMaxReplayJournalEntries = 4096)
        : MaxReplayJournalEntries(FMath::Max(1, InMaxReplayJournalEntries))
    {
    }

    int32 GetSchemaVersion() const { return SchemaVersion; }
    bool InstallPart(FName SlotId, const FString& StablePartId)
    {
        const FString Trimmed = StablePartId.TrimStartAndEnd();
        if (SlotId.IsNone() || Trimmed.IsEmpty()) return false;
        InstalledParts.Add(SlotId, Trimmed);
        return true;
    }

    EPinkCabPartInstallResult TryInstallPartOnce(
        const FPinkCabStableId& OperationId,
        const FPinkCabPartDefinition& Definition,
        FName VehicleCompatibilityTag)
    {
        if (!OperationId.IsValid() || !Definition.IsValid())
            return EPinkCabPartInstallResult::InvalidDefinition;
        const FString OpKey = OperationId.Serialize();
        if (AppliedInstallOperationIds.Contains(OpKey)) return EPinkCabPartInstallResult::Duplicate;
        if (VehicleCompatibilityTag.IsNone() || VehicleCompatibilityTag != Definition.CompatibilityTag)
            return EPinkCabPartInstallResult::Incompatible;
        if (AppliedInstallOperationIds.Num() >= MaxReplayJournalEntries)
            return EPinkCabPartInstallResult::CapacityExceeded;
        if (!InstallPart(Definition.SlotId, Definition.PartId))
            return EPinkCabPartInstallResult::InvalidDefinition;
        AppliedInstallOperationIds.Add(OpKey);
        return EPinkCabPartInstallResult::Applied;
    }

    FString GetInstalledPartId(FName SlotId) const
    {
        const FString* Found = InstalledParts.Find(SlotId);
        return Found ? *Found : FString();
    }

    int32 GetAppliedInstallCount() const { return AppliedInstallOperationIds.Num(); }
    int32 GetMaxReplayJournalEntries() const { return MaxReplayJournalEntries; }

private:
    friend class FPinkCabServiceSnapshotCodec;

    int32 SchemaVersion = 1;
    int32 MaxReplayJournalEntries = 4096;
    TMap<FName, FString> InstalledParts;
    TSet<FString> AppliedInstallOperationIds;
};
