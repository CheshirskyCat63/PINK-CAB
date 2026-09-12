#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabCityIdentity.h"

struct FPinkCabChunkCoord
{
    int32 Longitudinal = 0;
    int32 Lateral = 0;
    int32 Layer = 0;
};

struct FPinkCabChunkId
{
    FPinkCabChunkId() = default;
    explicit FPinkCabChunkId(const FString& InValue) : Value(InValue) {}

    static FPinkCabChunkId From(const FPinkCabCityIdentity& City, const FPinkCabChunkCoord& Coord)
    {
        const FString Payload = FString::Printf(
            TEXT("%s|%d|%d|%d"),
            *City.GetStableKey(), Coord.Longitudinal, Coord.Lateral, Coord.Layer);
        return FPinkCabChunkId(PinkCabWorldId::StableToken(TEXT("chunk:"), Payload));
    }
    bool IsValid() const { return !Value.IsEmpty(); }
    const FString& Serialize() const { return Value; }
    bool operator==(const FPinkCabChunkId& Other) const { return Value == Other.Value; }
    bool operator!=(const FPinkCabChunkId& Other) const { return !(*this == Other); }

private:
    FString Value;
};
