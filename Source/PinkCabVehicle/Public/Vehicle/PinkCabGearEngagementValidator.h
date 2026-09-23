#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabGearboxTypes.h"

struct FPinkCabGearEngagementDecision
{
    bool bAccepted = false;
    EPinkCabGearEngagementResult Result = EPinkCabGearEngagementResult::None;
    float ExpectedCoupledRpm = 0.0f;
};

class PINKCABVEHICLE_API FPinkCabGearEngagementValidator
{
public:
    static float ComputeClutchCoupling(
        const FPinkCabGearboxControllerConfig& Config,
        float ClutchPedal);
    static float ExpectedEngineRpmForGear(
        const FPinkCabGearboxControllerConfig& Config,
        int32 Gear,
        float SpeedKmh);
    static FPinkCabGearEngagementDecision EvaluateRequest(
        const FPinkCabGearboxControllerConfig& Config,
        int32 Gear,
        const FPinkCabGearEngagementContext& Context);
    static bool CanRetryPending(
        const FPinkCabGearboxControllerConfig& Config,
        int32 RequestedGear,
        const FPinkCabGearEngagementContext& Context);
    static bool IsDangerousOverrev(
        const FPinkCabGearboxControllerConfig& Config,
        int32 EngagedGear,
        const FPinkCabGearEngagementContext& Context,
        float ExpectedCoupledRpm);
};
