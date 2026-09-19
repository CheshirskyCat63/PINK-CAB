#pragma once

#include "CoreMinimal.h"

struct FPinkCabStableId
{
    FPinkCabStableId() = default;
    explicit FPinkCabStableId(const FString& InValue)
        : Value(InValue)
    {
    }

    bool IsValid() const
    {
        return !Value.IsEmpty();
    }

    const FString& Serialize() const
    {
        return Value;
    }

    bool operator==(const FPinkCabStableId& Other) const { return Value == Other.Value; }
    bool operator!=(const FPinkCabStableId& Other) const { return !(*this == Other); }

    static bool TryParse(const FString& Serialized, FPinkCabStableId& OutId)
    {
        const FString Normalized = Serialized.TrimStartAndEnd();
        if (Normalized.IsEmpty() || Normalized.Contains(TEXT("\n")) || Normalized.Contains(TEXT("\r")))
        {
            return false;
        }

        OutId = FPinkCabStableId(Normalized);
        return true;
    }

private:
    FString Value;
};
