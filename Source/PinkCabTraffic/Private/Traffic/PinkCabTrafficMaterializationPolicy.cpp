#include "Traffic/PinkCabTrafficMaterializationPolicy.h"

bool FPinkCabTrafficMaterializationPolicy::TryBuildRequest(
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
