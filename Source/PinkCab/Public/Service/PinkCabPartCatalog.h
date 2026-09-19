#pragma once

#include "CoreMinimal.h"

struct PINKCAB_API FPinkCabPartDefinition
{
    FString PartId;
    FName SlotId;
    int64 PriceMinor = 0;
    FName CompatibilityTag;

    bool IsValid() const;
};

class PINKCAB_API FPinkCabPartCatalog
{
public:
    explicit FPinkCabPartCatalog(int32 InMaxParts = 256);

    bool TryAdd(const FPinkCabPartDefinition& Definition);
    bool TryGet(
        const FString& PartId,
        FPinkCabPartDefinition& OutDefinition) const;
    int32 Num() const;

private:
    int32 MaxParts = 256;
    TMap<FString, FPinkCabPartDefinition> Definitions;
};
