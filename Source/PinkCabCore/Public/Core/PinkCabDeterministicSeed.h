#pragma once

#include "CoreMinimal.h"
#include "Containers/StringConv.h"
#include "Core/PinkCabStableId.h"

struct FPinkCabDeterministicSeed
{
    static uint64 FromText(const FString& Text)
    {
        uint64 Hash = OffsetBasis;
        MixText(Hash, Text);
        return EnsureNonZero(Hash);
    }

    static uint64 Derive(uint64 RootSeed, const FPinkCabStableId& StableId, const FString& Domain)
    {
        uint64 Hash = OffsetBasis;
        for (int32 Shift = 0; Shift < 64; Shift += 8)
        {
            MixByte(Hash, static_cast<uint8>((RootSeed >> Shift) & 0xFFull));
        }
        MixByte(Hash, 0xFFu);
        MixText(Hash, StableId.Serialize());
        MixByte(Hash, 0xFEu);
        MixText(Hash, Domain);
        return EnsureNonZero(Hash);
    }

private:
    static constexpr uint64 OffsetBasis = 14695981039346656037ull;
    static constexpr uint64 Prime = 1099511628211ull;

    static void MixByte(uint64& Hash, uint8 Byte)
    {
        Hash ^= static_cast<uint64>(Byte);
        Hash *= Prime;
    }

    static void MixText(uint64& Hash, const FString& Text)
    {
        FTCHARToUTF8 Utf8(*Text);
        const ANSICHAR* Data = Utf8.Get();
        for (int32 Index = 0; Index < Utf8.Length(); ++Index)
        {
            MixByte(Hash, static_cast<uint8>(Data[Index]));
        }
    }

    static uint64 EnsureNonZero(uint64 Seed)
    {
        return Seed == 0 ? OffsetBasis : Seed;
    }
};