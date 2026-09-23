#include "World/PinkCabL1EndlessRoadModel.h"

int32 FPinkCabL1EndlessRoadModel::ResolveChunkIndex(const double LongitudinalCm)
{
    return FMath::FloorToInt(LongitudinalCm / ChunkLengthCm);
}

EPinkCabLongitudinalTravelDirection FPinkCabL1EndlessRoadModel::ResolveTravelDirection(
    const double LongitudinalVelocityCmPerSec,
    const EPinkCabLongitudinalTravelDirection LastStableDirection,
    const double DirectionThresholdCmPerSec)
{
    const double Threshold = FMath::Abs(DirectionThresholdCmPerSec);
    if (LongitudinalVelocityCmPerSec > Threshold)
    {
        return EPinkCabLongitudinalTravelDirection::Positive;
    }
    if (LongitudinalVelocityCmPerSec < -Threshold)
    {
        return EPinkCabLongitudinalTravelDirection::Negative;
    }
    return LastStableDirection;
}

FPinkCabL1EndlessRoadWindow FPinkCabL1EndlessRoadModel::BuildWindow(
    const int32 CurrentChunkIndex,
    const EPinkCabLongitudinalTravelDirection Direction)
{
    FPinkCabL1EndlessRoadWindow Result;
    Result.CurrentChunkIndex = CurrentChunkIndex;
    Result.Direction = Direction;
    Result.DesiredCoords.Reserve(PoolSize);

    if (Direction == EPinkCabLongitudinalTravelDirection::Positive)
    {
        for (int32 Offset = -BehindCount; Offset <= AheadCount; ++Offset)
        {
            Result.DesiredCoords.Add({CurrentChunkIndex + Offset, 0, 0});
        }
    }
    else
    {
        for (int32 Offset = BehindCount; Offset >= -AheadCount; --Offset)
        {
            Result.DesiredCoords.Add({CurrentChunkIndex + Offset, 0, 0});
        }
    }

    return Result;
}
