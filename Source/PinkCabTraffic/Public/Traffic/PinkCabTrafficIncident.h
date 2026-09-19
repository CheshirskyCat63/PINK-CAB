#pragma once

#include "CoreMinimal.h"
#include "Traffic/PinkCabTrafficEntity.h"
#include "World/PinkCabCityDeltaState.h"
#include "World/PinkCabRouteService.h"

class PINKCABTRAFFIC_API FPinkCabTrafficIncidentRegistry
{
public:
    explicit FPinkCabTrafficIncidentRegistry(int32 MaxIncidents = 128);

    bool TryAddBlockedLane(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        const FString& IncidentKey);
    bool IsLaneBlocked(const FPinkCabLaneId& LaneId) const;
    const FPinkCabCityDeltaState& GetDeltaState() const;

private:
    FPinkCabCityDeltaState DeltaState;
};

struct PINKCABTRAFFIC_API FPinkCabTrafficBypassService
{
    static bool TryRerouteAtBoundary(
        FPinkCabTrafficEntity& Entity,
        const FPinkCabRoadGraph& Graph,
        const FPinkCabTrafficIncidentRegistry& Incidents,
        const FPinkCabLaneId& GoalLaneId,
        int32 MaxVisitedNodes);
};
