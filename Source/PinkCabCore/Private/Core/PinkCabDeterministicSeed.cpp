#include "Core/PinkCabDeterministicSeed.h"

#include "Containers/StringConv.h"

uint64 FPinkCabDeterministicSeed::FromText(const FString& Text)
{
    uint64 Hash = OffsetBasis;
    MixText(Hash, Text);
    return EnsureNonZero(Hash);
}

uint64 FPinkCabDeterministicSeed::Derive(
    uint64 RootSeed,
    const FPinkCabStableId& StableId,
    const FString& Domain)
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

void FPinkCabDeterministicSeed::MixByte(uint64& Hash, uint8 Byte)
{
    Hash ^= static_cast<uint64>(Byte);
    Hash *= Prime;
}

void FPinkCabDeterministicSeed::MixText(
    uint64& Hash,
    const FString& Text)
{
    FTCHARToUTF8 Utf8(*Text);
    const ANSICHAR* Data = Utf8.Get();
    for (int32 Index = 0; Index < Utf8.Length(); ++Index)
    {
        MixByte(Hash, static_cast<uint8>(Data[Index]));
    }
}

uint64 FPinkCabDeterministicSeed::EnsureNonZero(uint64 Seed)
{
    return Seed == 0 ? OffsetBasis : Seed;
}
