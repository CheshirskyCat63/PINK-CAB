#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabRoadGraph.h"

struct FPinkCabTrafficEntity
{
    FPinkCabTrafficEntity() = default;
    FPinkCabTrafficEntity(
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

    bool IsValidLogical() const
    {
        return !TrafficId.IsEmpty() && LaneId.IsValid()
            && FMath::IsFinite(LongitudinalCm) && LongitudinalCm >= 0.0
            && FMath::IsFinite(SpeedCmPerSec);
    }

    bool TryAssignRoute(const TArray<FPinkCabLaneId>& InRoute)
    {
        if (InRoute.IsEmpty() || !(InRoute[0] == LaneId))
        {
            return false;
        }        for (const FPinkCabLaneId& RouteLane : InRoute)
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

    bool AdvanceAlongRoute(const FPinkCabRoadGraph& Graph, double DeltaSeconds)
    {
        if (DeltaSeconds <= 0.0 || !FMath::IsFinite(DeltaSeconds)
            || SpeedCmPerSec <= 0.0 || !FMath::IsFinite(SpeedCmPerSec))
        {
            return false;
        }

        double RemainingDistance = SpeedCmPerSec * DeltaSeconds;
        while (RemainingDistance > 0.0)
        {
            const FPinkCabLogicalLane* CurrentLane = Graph.FindLane(LaneId);
            if (!CurrentLane)
            {
                return false;
            }
            const double ToBoundary = FMath::Max(0.0, CurrentLane->LengthCm - LongitudinalCm);
            if (RemainingDistance < ToBoundary)
            {
                LongitudinalCm += RemainingDistance;
                return true;
            }            RemainingDistance -= ToBoundary;
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

    void Advance(double DeltaSeconds)
    {
        if (DeltaSeconds > 0.0 && FMath::IsFinite(DeltaSeconds))
        {
            LongitudinalCm += SpeedCmPerSec * DeltaSeconds;
        }
    }

    bool IsAtLaneBoundary(const FPinkCabRoadGraph& Graph) const
    {
        const FPinkCabLogicalLane* Lane = Graph.FindLane(LaneId);
        return Lane && FMath::IsNearlyEqual(LongitudinalCm, Lane->LengthCm);
    }

    double GetLongitudinalCm() const { return LongitudinalCm; }
    double GetSpeedCmPerSec() const { return SpeedCmPerSec; }
    const FString& GetTrafficId() const { return TrafficId; }
    const FPinkCabLaneId& GetLaneId() const { return LaneId; }
    int32 GetRouteCursor() const { return RouteCursor; }

private:    FString TrafficId;
    FPinkCabLaneId LaneId;
    double LongitudinalCm = 0.0;
    double SpeedCmPerSec = 0.0;
    TArray<FPinkCabLaneId> RouteLaneIds;
    int32 RouteCursor = INDEX_NONE;
};
