#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabChunkId.h"

enum class EPinkCabLongitudinalTravelDirection : int8
{
    Negative = -1,
    Positive = 1
};

struct PINKCABWORLD_API FPinkCabL1EndlessRoadWindow
{
    int32 CurrentChunkIndex = 0;
    EPinkCabLongitudinalTravelDirection Direction =
        EPinkCabLongitudinalTravelDirection::Positive;
    TArray<FPinkCabChunkCoord> DesiredCoords;
};

class PINKCABWORLD_API FPinkCabL1EndlessRoadModel
{
public:
    static constexpr double ChunkLengthCm = 100000.0;
    static constexpr int32 PoolSize = 7;
    static constexpr int32 BehindCount = 2;
    static constexpr int32 AheadCount = 4;
    static constexpr int32 GroundLaneCount = 14;

    static int32 ResolveChunkIndex(double LongitudinalCm);

    static EPinkCabLongitudinalTravelDirection ResolveTravelDirection(
        double LongitudinalVelocityCmPerSec,
        EPinkCabLongitudinalTravelDirection LastStableDirection,
        double DirectionThresholdCmPerSec);

    static FPinkCabL1EndlessRoadWindow BuildWindow(
        int32 CurrentChunkIndex,
        EPinkCabLongitudinalTravelDirection Direction);
};
