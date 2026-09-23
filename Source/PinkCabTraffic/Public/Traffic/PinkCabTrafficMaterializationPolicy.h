#pragma once

#include "CoreMinimal.h"
#include "Traffic/PinkCabTrafficEntity.h"

struct FPinkCabTrafficMaterializationRequest
{
    FString TrafficId;
    FPinkCabLaneId LaneId;
    double LongitudinalCm = 0.0;
};

struct PINKCABTRAFFIC_API FPinkCabTrafficMaterializationPolicy
{
    static bool TryBuildRequest(
        const FPinkCabTrafficEntity& Entity,
        bool bExternalPolicyWantsMaterialization,
        FPinkCabTrafficMaterializationRequest& OutRequest);
};
