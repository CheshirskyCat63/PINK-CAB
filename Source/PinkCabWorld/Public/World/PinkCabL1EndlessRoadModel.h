#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabChunkId.h"

class FPinkCabRoadGraph;

enum class EPinkCabLongitudinalTravelDirection : int8
{
    Negative = -1,
    Positive = 1
};

enum class EPinkCabL1AccessRole : uint8
{
    None,
    ExitToLocal,
    MergeToExpress
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

    // R1 Road V2 access topology. The two access windows live inside the
    // 1000m module while the first/last 150m remain clean seam buffers.
    static constexpr int32 AccessWindowCount = 2;
    static constexpr double AccessConnectorMaxWidthCm = 400.0;

    static constexpr double AccessAStartCm = 15000.0;
    static constexpr double AccessAFullOpenStartCm = 27500.0;
    static constexpr double AccessAFullOpenEndCm = 32500.0;
    static constexpr double AccessAEndCm = 45000.0;

    static constexpr double AccessBStartCm = 55000.0;
    static constexpr double AccessBFullOpenStartCm = 67500.0;
    static constexpr double AccessBFullOpenEndCm = 72500.0;
    static constexpr double AccessBEndCm = 85000.0;

    static int32 ResolveChunkIndex(double LongitudinalCm);

    static EPinkCabLongitudinalTravelDirection ResolveTravelDirection(
        double LongitudinalVelocityCmPerSec,
        EPinkCabLongitudinalTravelDirection LastStableDirection,
        double DirectionThresholdCmPerSec);

    static FPinkCabL1EndlessRoadWindow BuildWindow(
        int32 CurrentChunkIndex,
        EPinkCabLongitudinalTravelDirection Direction);

    static double ResolveAccessConnectorWidthCm(double LocalLongitudinalCm);
    static double ResolveAccessSeparatorWidthCm(double LocalLongitudinalCm);
    static EPinkCabL1AccessRole ResolveAccessRole(
        int32 PhysicalWindowIndex,
        EPinkCabLongitudinalTravelDirection Direction);

    static bool AppendStraightChunkLanes(
        const FPinkCabCityIdentity& City,
        int32 ChunkIndex,
        FPinkCabRoadGraph& InOutGraph);
};
