#pragma once

#include "CoreMinimal.h"

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

    FString GetInstalledPartId(FName SlotId) const
    {
        const FString* Found = InstalledParts.Find(SlotId);
        return Found ? *Found : FString();
    }

private:
    int32 SchemaVersion = 1;
    TMap<FName, FString> InstalledParts;
};
