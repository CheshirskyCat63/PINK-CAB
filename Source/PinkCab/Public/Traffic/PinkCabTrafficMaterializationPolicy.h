#pragma once

#include "CoreMinimal.h"
#include "Traffic/PinkCabTrafficEntity.h"

struct FPinkCabTrafficMaterializationRequest
{
    FString TrafficId;
    FPinkCabLaneId LaneId;
    double LongitudinalCm = 0.0;
};

struct FPinkCabTrafficMaterializationPolicy
{
    static bool TryBuildRequest(
        const FPinkCabTrafficEntity& Entity,
        bool bExternalPolicyWantsMaterialization,
        FPinkCabTrafficMaterializationRequest& OutRequest)
    {
        if (!bExternalPolicyWantsMaterialization || !Entity.IsValidLogical())
        {
            return false;
        }
        OutRequest.TrafficId = Entity.GetTrafficId();
        OutRequest.LaneId = Entity.GetLaneId();
        OutRequest.LongitudinalCm = Entity.GetLongitudinalCm();
        return true;
    }
};
