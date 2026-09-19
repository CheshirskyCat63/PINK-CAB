#pragma once

#include "CoreMinimal.h"

struct PINKCABCORE_API FPinkCabSchemaVersion
{
    FPinkCabSchemaVersion() = default;
    FPinkCabSchemaVersion(int32 InMajor, int32 InMinor, int32 InPatch);

    bool IsValid() const;
    FString Serialize() const;
    bool operator==(const FPinkCabSchemaVersion& Other) const;
    bool operator<(const FPinkCabSchemaVersion& Other) const;

    static bool TryParse(
        const FString& Text,
        FPinkCabSchemaVersion& OutVersion);

private:
    int32 Major = 0;
    int32 Minor = 0;
    int32 Patch = 0;
};
