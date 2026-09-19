#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabChunkId.h"
#include "World/PinkCabCityLocationRegistry.h"

struct PINKCABWORLD_API FPinkCabWorldChunkCandidate
{
    FPinkCabChunkId ChunkId;
    FPinkCabChunkCoord Coord;
    bool IsValid() const;
};

struct PINKCABWORLD_API FPinkCabWorldMaterializationSettings
{
    int32 MaxMaterializedChunks = 0;
    int64 MaxDistanceSquared = 0;
    TArray<int32> AllowedLayers;
    int32 MaxAnchorRequests = 0;
    bool IsValid() const;
};

struct PINKCABWORLD_API FPinkCabWorldMaterializationResult
{
    TArray<FPinkCabChunkId> MaterializeChunkIds;
    TArray<FPinkCabChunkId> DematerializeChunkIds;
    FString RequestSignature;
    void Reset();
};

class PINKCABWORLD_API FPinkCabWorldMaterializationPolicy
{
public:
    static bool BuildWindow(
        const FPinkCabChunkCoord& PlayerCoord,
        const TArray<FPinkCabWorldChunkCandidate>& Candidates,
        const FPinkCabWorldMaterializationSettings& Settings,
        const TArray<FPinkCabChunkId>& PreviousMaterialized,
        FPinkCabWorldMaterializationResult& OutResult);
    static bool BuildAnchorRequests(
        const FPinkCabCityLocationRegistry& Registry,
        const FPinkCabWorldMaterializationSettings& Settings,
        TArray<FString>& OutAnchorIds);

private:
    struct FScoredCandidate
    {
        FPinkCabChunkId ChunkId;
        int64 DistanceSquared = 0;
    };
};
