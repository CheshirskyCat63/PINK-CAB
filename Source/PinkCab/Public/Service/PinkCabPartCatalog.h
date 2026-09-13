#pragma once

#include "CoreMinimal.h"

struct FPinkCabPartDefinition
{
    FString PartId;
    FName SlotId;
    int64 PriceMinor = 0;
    FName CompatibilityTag;

    bool IsValid() const
    {
        return !PartId.TrimStartAndEnd().IsEmpty() && !SlotId.IsNone()
            && PriceMinor > 0 && !CompatibilityTag.IsNone();
    }
};

class FPinkCabPartCatalog
{
public:
    explicit FPinkCabPartCatalog(int32 InMaxParts = 256)
        : MaxParts(FMath::Max(1, InMaxParts)) {}

    bool TryAdd(const FPinkCabPartDefinition& Definition)
    {
        const FString Key = Definition.PartId.TrimStartAndEnd();
        if (!Definition.IsValid() || Definitions.Num() >= MaxParts || Definitions.Contains(Key))
            return false;
        FPinkCabPartDefinition Clean = Definition;
        Clean.PartId = Key;
        Definitions.Add(Key, Clean);
        return true;
    }

    bool TryGet(const FString& PartId, FPinkCabPartDefinition& OutDefinition) const
    {
        const FPinkCabPartDefinition* Found = Definitions.Find(PartId.TrimStartAndEnd());
        if (!Found) return false;
        OutDefinition = *Found;
        return true;
    }

    int32 Num() const { return Definitions.Num(); }

private:
    int32 MaxParts = 256;
    TMap<FString, FPinkCabPartDefinition> Definitions;
};
