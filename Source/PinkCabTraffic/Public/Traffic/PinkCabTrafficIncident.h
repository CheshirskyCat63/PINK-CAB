#pragma once

#include "CoreMinimal.h"
#include "Traffic/PinkCabTrafficEntity.h"
#include "World/PinkCabCityDeltaState.h"
#include "World/PinkCabRouteService.h"

class FPinkCabTrafficIncidentRegistry
{
public:
    explicit FPinkCabTrafficIncidentRegistry(int32 MaxIncidents = 128)
        : DeltaState(MaxIncidents)
    {
    }

    bool TryAddBlockedLane(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        const FString& IncidentKey)
    {
        FString DeltaId;
        return DeltaState.TryAddLaneClosure(City, LaneId, IncidentKey, DeltaId);
    }

    bool IsLaneBlocked(const FPinkCabLaneId& LaneId) const
    {
        return DeltaState.IsLaneClosed(LaneId);
    }

    const FPinkCabCityDeltaState& GetDeltaState() const
    {
        return DeltaState;
    }
private:
    FPinkCabCityDeltaState DeltaState;
};

struct FPinkCabTrafficBypassService
{
    static bool TryRerouteAtBoundary(
        FPinkCabTrafficEntity& Entity,
        const FPinkCabRoadGraph& Graph,
        const FPinkCabTrafficIncidentRegistry& Incidents,
        const FPinkCabLaneId& GoalLaneId,
        int32 MaxVisitedNodes)
    {
        if (!Entity.IsAtLaneBoundary(Graph) || Entity.GetSpeedCmPerSec() <= 0.0)
        {
            return false;
        }

        FPinkCabRouteRequest Request;
        Request.StartLaneId = Entity.GetLaneId();
        Request.GoalLaneId = GoalLaneId;
        Request.MaxVisitedNodes = MaxVisitedNodes;
        FPinkCabRoute Route;
        if (!FPinkCabRouteService::FindRoute(Graph, Request, Incidents.GetDeltaState(), Route))
        {
            return false;
        }
        return Entity.TryAssignRoute(Route.LaneIds);
    }
};
