#include "World/PinkCabL1EndlessRoadModel.h"

#include "World/PinkCabRoadGraph.h"

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


namespace
{
double ResolveAccessWindowWidth(
    const double X,
    const double Start,
    const double FullOpenStart,
    const double FullOpenEnd,
    const double End)
{
    if (X <= Start || X >= End)
    {
        return 0.0;
    }
    if (X < FullOpenStart)
    {
        const double Alpha = (X - Start) / (FullOpenStart - Start);
        return FMath::Clamp(
            Alpha * FPinkCabL1EndlessRoadModel::AccessConnectorMaxWidthCm,
            0.0,
            FPinkCabL1EndlessRoadModel::AccessConnectorMaxWidthCm);
    }
    if (X <= FullOpenEnd)
    {
        return FPinkCabL1EndlessRoadModel::AccessConnectorMaxWidthCm;
    }

    const double Alpha = (End - X) / (End - FullOpenEnd);
    return FMath::Clamp(
        Alpha * FPinkCabL1EndlessRoadModel::AccessConnectorMaxWidthCm,
        0.0,
        FPinkCabL1EndlessRoadModel::AccessConnectorMaxWidthCm);
}
}

double FPinkCabL1EndlessRoadModel::ResolveAccessConnectorWidthCm(
    const double LocalLongitudinalCm)
{
    if (!FMath::IsFinite(LocalLongitudinalCm)
        || LocalLongitudinalCm < 0.0
        || LocalLongitudinalCm > ChunkLengthCm)
    {
        return 0.0;
    }

    const double A = ResolveAccessWindowWidth(
        LocalLongitudinalCm,
        AccessAStartCm,
        AccessAFullOpenStartCm,
        AccessAFullOpenEndCm,
        AccessAEndCm);
    const double B = ResolveAccessWindowWidth(
        LocalLongitudinalCm,
        AccessBStartCm,
        AccessBFullOpenStartCm,
        AccessBFullOpenEndCm,
        AccessBEndCm);
    return FMath::Max(A, B);
}

double FPinkCabL1EndlessRoadModel::ResolveAccessSeparatorWidthCm(
    const double LocalLongitudinalCm)
{
    return AccessConnectorMaxWidthCm
        - ResolveAccessConnectorWidthCm(LocalLongitudinalCm);
}

EPinkCabL1AccessRole FPinkCabL1EndlessRoadModel::ResolveAccessRole(
    const int32 PhysicalWindowIndex,
    const EPinkCabLongitudinalTravelDirection Direction)
{
    if (PhysicalWindowIndex < 0 || PhysicalWindowIndex >= AccessWindowCount)
    {
        return EPinkCabL1AccessRole::None;
    }

    const bool bFirstPhysicalWindow = PhysicalWindowIndex == 0;
    if (Direction == EPinkCabLongitudinalTravelDirection::Positive)
    {
        return bFirstPhysicalWindow
            ? EPinkCabL1AccessRole::ExitToLocal
            : EPinkCabL1AccessRole::MergeToExpress;
    }

    return bFirstPhysicalWindow
        ? EPinkCabL1AccessRole::MergeToExpress
        : EPinkCabL1AccessRole::ExitToLocal;
}



namespace PinkCabL1EndlessRoadTopology
{
struct FLaneSpec
{
    int32 RoadIndex = 0;
    int32 LaneIndex = 0;
    int32 BoundaryNodeIndex = 0;
    bool bPositive = true;
};

TArray<FLaneSpec> BuildLaneSpecs()
{
    TArray<FLaneSpec> Specs;
    Specs.Reserve(FPinkCabL1EndlessRoadModel::GroundLaneCount);

    for (int32 Lane = 0; Lane < 5; ++Lane)
    {
        Specs.Add({0, Lane, Lane, true});
    }
    for (int32 Lane = 0; Lane < 5; ++Lane)
    {
        Specs.Add({1, Lane, 5 + Lane, false});
    }
    for (int32 Lane = 0; Lane < 2; ++Lane)
    {
        Specs.Add({2, Lane, 10 + Lane, true});
    }
    for (int32 Lane = 0; Lane < 2; ++Lane)
    {
        Specs.Add({3, Lane, 12 + Lane, false});
    }
    return Specs;
}
}

bool FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(
    const FPinkCabCityIdentity& City,
    const int32 ChunkIndex,
    FPinkCabRoadGraph& InOutGraph)
{
    if (!City.IsValid())
    {
        return false;
    }

    const FPinkCabChunkId RoadChunk = FPinkCabChunkId::From(City, {ChunkIndex, 0, 0});
    const FPinkCabChunkId StartBoundary = FPinkCabChunkId::From(City, {ChunkIndex, 0, 0});
    const FPinkCabChunkId EndBoundary = FPinkCabChunkId::From(City, {ChunkIndex + 1, 0, 0});
    const TArray<PinkCabL1EndlessRoadTopology::FLaneSpec> Specs =
        PinkCabL1EndlessRoadTopology::BuildLaneSpecs();

    TArray<FPinkCabLogicalLane> Pending;
    Pending.Reserve(Specs.Num());

    for (const PinkCabL1EndlessRoadTopology::FLaneSpec& Spec : Specs)
    {
        FPinkCabLogicalLane Lane;
        Lane.LaneId = FPinkCabRoadGraph::MakeLaneId(
            City, RoadChunk, Spec.RoadIndex, Spec.LaneIndex);

        const FPinkCabRoadNodeId StartNode = FPinkCabRoadGraph::MakeNodeId(
            City, StartBoundary, Spec.BoundaryNodeIndex);
        const FPinkCabRoadNodeId EndNode = FPinkCabRoadGraph::MakeNodeId(
            City, EndBoundary, Spec.BoundaryNodeIndex);

        Lane.FromNode = Spec.bPositive ? StartNode : EndNode;
        Lane.ToNode = Spec.bPositive ? EndNode : StartNode;
        Lane.LengthCm = ChunkLengthCm;
        Lane.Layer = 0;

        if (InOutGraph.FindLane(Lane.LaneId) != nullptr)
        {
            return false;
        }
        Pending.Add(MoveTemp(Lane));
    }

    for (const FPinkCabLogicalLane& Lane : Pending)
    {
        if (!InOutGraph.AddLane(Lane))
        {
            return false;
        }
    }
    return Pending.Num() == GroundLaneCount;
}
