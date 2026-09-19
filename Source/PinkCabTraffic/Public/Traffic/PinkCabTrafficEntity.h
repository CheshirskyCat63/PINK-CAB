#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabRoadGraph.h"

struct PINKCABTRAFFIC_API FPinkCabTrafficEntity
{
    FPinkCabTrafficEntity();
    FPinkCabTrafficEntity(
        const FString& InTrafficId,
        const FPinkCabLaneId& InLaneId,
        double InLongitudinalCm,
        double InSpeedCmPerSec);

    bool IsValidLogical() const;
    bool TryAssignRoute(const TArray<FPinkCabLaneId>& InRoute);
    bool AdvanceAlongRoute(const FPinkCabRoadGraph& Graph, double DeltaSeconds);
    void Advance(double DeltaSeconds);
    bool IsAtLaneBoundary(const FPinkCabRoadGraph& Graph) const;

    double GetLongitudinalCm() const;
    double GetSpeedCmPerSec() const;
    const FString& GetTrafficId() const;
    const FPinkCabLaneId& GetLaneId() const;
    int32 GetRouteCursor() const;

private:
    FString TrafficId;
    FPinkCabLaneId LaneId;
    double LongitudinalCm = 0.0;
    double SpeedCmPerSec = 0.0;
    TArray<FPinkCabLaneId> RouteLaneIds;
    int32 RouteCursor = INDEX_NONE;
};
