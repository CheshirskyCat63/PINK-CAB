#include "Traffic/PinkCabTrafficEntity.h"

FPinkCabTrafficEntity::FPinkCabTrafficEntity() = default;

FPinkCabTrafficEntity::FPinkCabTrafficEntity(
    const FString& InTrafficId,
    const FPinkCabLaneId& InLaneId,
    double InLongitudinalCm,
    double InSpeedCmPerSec)
    : TrafficId(InTrafficId)
    , LaneId(InLaneId)
    , LongitudinalCm(InLongitudinalCm)
    , SpeedCmPerSec(InSpeedCmPerSec)
{
}

bool FPinkCabTrafficEntity::IsValidLogical() const
{
    return !TrafficId.IsEmpty()
        && LaneId.IsValid()
        && FMath::IsFinite(LongitudinalCm)
        && LongitudinalCm >= 0.0
        && FMath::IsFinite(SpeedCmPerSec);
}

bool FPinkCabTrafficEntity::TryAssignRoute(
    const TArray<FPinkCabLaneId>& InRoute)
{
    if (InRoute.IsEmpty() || !(InRoute[0] == LaneId))
    {
        return false;
    }
    for (const FPinkCabLaneId& RouteLane : InRoute)
    {
        if (!RouteLane.IsValid())
        {
            return false;
        }
    }
    RouteLaneIds = InRoute;
    RouteCursor = 0;
    return true;
}

bool FPinkCabTrafficEntity::AdvanceAlongRoute(
    const FPinkCabRoadGraph& Graph,
    double DeltaSeconds)
{
    if (DeltaSeconds <= 0.0
        || !FMath::IsFinite(DeltaSeconds)
        || SpeedCmPerSec <= 0.0
        || !FMath::IsFinite(SpeedCmPerSec))
    {
        return false;
    }

    double RemainingDistance = SpeedCmPerSec * DeltaSeconds;
    while (RemainingDistance > 0.0)
    {
        const FPinkCabLogicalLane* CurrentLane = Graph.FindLane(LaneId);
        if (!CurrentLane) return false;

        const double ToBoundary =
            FMath::Max(0.0, CurrentLane->LengthCm - LongitudinalCm);
        if (RemainingDistance < ToBoundary)
        {
            LongitudinalCm += RemainingDistance;
            return true;
        }

        RemainingDistance -= ToBoundary;
        LongitudinalCm = CurrentLane->LengthCm;
        if (RouteCursor == INDEX_NONE || RouteCursor + 1 >= RouteLaneIds.Num())
        {
            return true;
        }

        ++RouteCursor;
        LaneId = RouteLaneIds[RouteCursor];
        LongitudinalCm = 0.0;
    }
    return true;
}

void FPinkCabTrafficEntity::Advance(double DeltaSeconds)
{
    if (DeltaSeconds > 0.0 && FMath::IsFinite(DeltaSeconds))
    {
        LongitudinalCm += SpeedCmPerSec * DeltaSeconds;
    }
}

bool FPinkCabTrafficEntity::IsAtLaneBoundary(
    const FPinkCabRoadGraph& Graph) const
{
    const FPinkCabLogicalLane* Lane = Graph.FindLane(LaneId);
    return Lane && FMath::IsNearlyEqual(LongitudinalCm, Lane->LengthCm);
}

double FPinkCabTrafficEntity::GetLongitudinalCm() const { return LongitudinalCm; }
double FPinkCabTrafficEntity::GetSpeedCmPerSec() const { return SpeedCmPerSec; }
const FString& FPinkCabTrafficEntity::GetTrafficId() const { return TrafficId; }
const FPinkCabLaneId& FPinkCabTrafficEntity::GetLaneId() const { return LaneId; }
int32 FPinkCabTrafficEntity::GetRouteCursor() const { return RouteCursor; }
