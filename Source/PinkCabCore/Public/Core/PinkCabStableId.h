#pragma once

#include "CoreMinimal.h"

struct PINKCABCORE_API FPinkCabStableId
{
    FPinkCabStableId() = default;
    explicit FPinkCabStableId(const FString& InValue);

    bool IsValid() const;
    const FString& Serialize() const;
    bool operator==(const FPinkCabStableId& Other) const;
    bool operator!=(const FPinkCabStableId& Other) const;

    static bool TryParse(
        const FString& Serialized,
        FPinkCabStableId& OutId);

private:
    FString Value;
};
