#include "Core/PinkCabStableId.h"

FPinkCabStableId::FPinkCabStableId(const FString& InValue)
    : Value(InValue)
{
}

bool FPinkCabStableId::IsValid() const
{
    return !Value.IsEmpty();
}

const FString& FPinkCabStableId::Serialize() const
{
    return Value;
}

bool FPinkCabStableId::operator==(
    const FPinkCabStableId& Other) const
{
    return Value == Other.Value;
}

bool FPinkCabStableId::operator!=(
    const FPinkCabStableId& Other) const
{
    return !(*this == Other);
}
bool FPinkCabStableId::TryParse(
    const FString& Serialized,
    FPinkCabStableId& OutId)
{
    const FString Normalized = Serialized.TrimStartAndEnd();
    if (Normalized.IsEmpty()
        || Normalized.Contains(TEXT("\n"))
        || Normalized.Contains(TEXT("\r")))
    {
        return false;
    }

    OutId = FPinkCabStableId(Normalized);
    return true;
}
