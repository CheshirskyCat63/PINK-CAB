#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct PINKCABCORE_API FPinkCabDeterministicSeed
{
    static uint64 FromText(const FString& Text);
    static uint64 Derive(
        uint64 RootSeed,
        const FPinkCabStableId& StableId,
        const FString& Domain);

private:
    static constexpr uint64 OffsetBasis = 14695981039346656037ull;
    static constexpr uint64 Prime = 1099511628211ull;

    static void MixByte(uint64& Hash, uint8 Byte);
    static void MixText(uint64& Hash, const FString& Text);
    static uint64 EnsureNonZero(uint64 Seed);
};
