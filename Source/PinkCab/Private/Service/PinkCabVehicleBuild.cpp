#include "Service/PinkCabVehicleBuild.h"

FPinkCabVehicleBuild::FPinkCabVehicleBuild(
    int32 InMaxReplayJournalEntries)
    : MaxReplayJournalEntries(
        FMath::Max(1, InMaxReplayJournalEntries))
{
}

int32 FPinkCabVehicleBuild::GetSchemaVersion() const
{
    return SchemaVersion;
}

bool FPinkCabVehicleBuild::InstallPart(
    FName SlotId,
    const FString& StablePartId)
{
    const FString Trimmed = StablePartId.TrimStartAndEnd();
    if (SlotId.IsNone() || Trimmed.IsEmpty())
    {
        return false;
    }
    InstalledParts.Add(SlotId, Trimmed);
    return true;
}

EPinkCabPartInstallResult FPinkCabVehicleBuild::TryInstallPartOnce(
    const FPinkCabStableId& OperationId,
    const FPinkCabPartDefinition& Definition,
    FName VehicleCompatibilityTag)
{
    if (!OperationId.IsValid() || !Definition.IsValid())
    {
        return EPinkCabPartInstallResult::InvalidDefinition;
    }

    const FString OpKey = OperationId.Serialize();
    if (AppliedInstallOperationIds.Contains(OpKey))
    {
        return EPinkCabPartInstallResult::Duplicate;
    }
    if (VehicleCompatibilityTag.IsNone()
        || VehicleCompatibilityTag != Definition.CompatibilityTag)
    {
        return EPinkCabPartInstallResult::Incompatible;
    }
    if (AppliedInstallOperationIds.Num()
        >= MaxReplayJournalEntries)
    {
        return EPinkCabPartInstallResult::CapacityExceeded;
    }
    if (!InstallPart(Definition.SlotId, Definition.PartId))
    {
        return EPinkCabPartInstallResult::InvalidDefinition;
    }

    AppliedInstallOperationIds.Add(OpKey);
    return EPinkCabPartInstallResult::Applied;
}

FString FPinkCabVehicleBuild::GetInstalledPartId(
    FName SlotId) const
{
    const FString* Found = InstalledParts.Find(SlotId);
    return Found ? *Found : FString();
}

int32 FPinkCabVehicleBuild::GetAppliedInstallCount() const
{
    return AppliedInstallOperationIds.Num();
}

int32 FPinkCabVehicleBuild::GetMaxReplayJournalEntries() const
{
    return MaxReplayJournalEntries;
}
