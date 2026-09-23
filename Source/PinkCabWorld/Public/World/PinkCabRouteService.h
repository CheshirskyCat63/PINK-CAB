#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabCityDeltaState.h"
#include "World/PinkCabRoadGraph.h"

struct PINKCABWORLD_API FPinkCabRouteRequest
{
    FPinkCabLaneId StartLaneId;
    FPinkCabLaneId GoalLaneId;
    int32 MaxVisitedNodes = 0;

    bool IsValid() const;
};

struct PINKCABWORLD_API FPinkCabRoute
{
    TArray<FPinkCabLaneId> LaneIds;
    double TotalLengthCm = 0.0;
    FString RouteSignature;

    void Reset();
};

class PINKCABWORLD_API FPinkCabRouteService
{
public:
    static bool FindRoute(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabRouteRequest& Request,
        const FPinkCabCityDeltaState& Deltas,
        FPinkCabRoute& OutRoute);

private:
    struct FFrontierEntry
    {
        FString LaneKey;
        double Cost = 0.0;
        FString PathKey;
    };

    static bool ValidateEndpoints(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabRouteRequest& Request,
        const FPinkCabCityDeltaState& Deltas,
        const FPinkCabLogicalLane*& OutStartLane);
    static bool SearchShortestPath(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabRouteRequest& Request,
        const FPinkCabCityDeltaState& Deltas,
        const FPinkCabLogicalLane& StartLane,
        const FString& StartKey,
        const FString& GoalKey,
        TMap<FString, double>& OutBestCost,
        TMap<FString, FString>& OutPrevious);
    static void ExpandFrontier(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabCityDeltaState& Deltas,
        const FFrontierEntry& Current,
        TArray<FFrontierEntry>& Frontier,
        TMap<FString, double>& BestCost,
        TMap<FString, FString>& BestPathKey,
        TMap<FString, FString>& Previous);
    static void TryRelaxLane(
        const FPinkCabLogicalLane& NextLane,
        const FFrontierEntry& Current,
        TArray<FFrontierEntry>& Frontier,
        TMap<FString, double>& BestCost,
        TMap<FString, FString>& BestPathKey,
        TMap<FString, FString>& Previous);
    static bool BuildRoute(
        const FString& StartKey,
        const FString& GoalKey,
        const TMap<FString, double>& BestCost,
        const TMap<FString, FString>& Previous,
        FPinkCabRoute& OutRoute);
    static int32 FindBestFrontierIndex(
        const TArray<FFrontierEntry>& Frontier);
};
