#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Service/PinkCabPartCatalog.h"

enum class EPinkCabPartInstallResult : uint8
{
    Applied,
    Duplicate,
    InvalidDefinition,
    Incompatible
};

class FPinkCabVehicleBuild
{
public:
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

private:
    int32 SchemaVersion = 1;
    TMap<FName, FString> InstalledParts;
    TSet<FString> AppliedInstallOperationIds;
};
