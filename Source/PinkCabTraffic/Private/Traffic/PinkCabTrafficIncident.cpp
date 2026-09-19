#include "Traffic/PinkCabTrafficIncident.h"

FPinkCabTrafficIncidentRegistry::FPinkCabTrafficIncidentRegistry(
    int32 MaxIncidents)
    : DeltaState(MaxIncidents)
{
}

bool FPinkCabTrafficIncidentRegistry::TryAddBlockedLane(
    const FPinkCabCityIdentity& City,
    const FPinkCabLaneId& LaneId,
    const FString& IncidentKey)
{
    FString DeltaId;
    return DeltaState.TryAddLaneClosure(City, LaneId, IncidentKey, DeltaId);
}

bool FPinkCabTrafficIncidentRegistry::IsLaneBlocked(
    const FPinkCabLaneId& LaneId) const
{
    return DeltaState.IsLaneClosed(LaneId);
}

const FPinkCabCityDeltaState& FPinkCabTrafficIncidentRegistry::GetDeltaState() const
{
    return DeltaState;
}

bool FPinkCabTrafficBypassService::TryRerouteAtBoundary(
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
    if (!FPinkCabRouteService::FindRoute(
        Graph,
        Request,
        Incidents.GetDeltaState(),
        Route))
    {
        return false;
    }
    return Entity.TryAssignRoute(Route.LaneIds);
}
