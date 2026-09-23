#include "Service/PinkCabPartCatalog.h"

bool FPinkCabPartDefinition::IsValid() const
{
    return !PartId.TrimStartAndEnd().IsEmpty()
        && !SlotId.IsNone()
        && PriceMinor > 0
        && !CompatibilityTag.IsNone();
}

FPinkCabPartCatalog::FPinkCabPartCatalog(int32 InMaxParts)
    : MaxParts(FMath::Max(1, InMaxParts))
{
}

bool FPinkCabPartCatalog::TryAdd(
    const FPinkCabPartDefinition& Definition)
{
    const FString Key = Definition.PartId.TrimStartAndEnd();
    if (!Definition.IsValid()
        || Definitions.Num() >= MaxParts
        || Definitions.Contains(Key))
    {
        return false;
    }

    FPinkCabPartDefinition Clean = Definition;
    Clean.PartId = Key;
    Definitions.Add(Key, Clean);
    return true;
}

bool FPinkCabPartCatalog::TryGet(
    const FString& PartId,
    FPinkCabPartDefinition& OutDefinition) const
{
    const FPinkCabPartDefinition* Found =
        Definitions.Find(PartId.TrimStartAndEnd());
    if (!Found)
    {
        return false;
    }
    OutDefinition = *Found;
    return true;
}

int32 FPinkCabPartCatalog::Num() const
{
    return Definitions.Num();
}
