#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabGearboxTypes.h"
#include "Vehicle/PinkCabHGateGeometry.h"

class PINKCABVEHICLE_API FPinkCabGearboxController
{
public:
    explicit FPinkCabGearboxController(const FPinkCabGearboxControllerConfig& InConfig = {});

    int32 GetRequestedGear() const { return GateState.RequestedGear; }
    int32 GetEngagedGear() const { return EngagedGear; }
    EPinkCabGearEngagementResult GetLastResult() const { return LastResult; }
    float GetExpectedCoupledRpm() const { return ExpectedCoupledRpm; }
    float GetMaxSafeEngineRpm() const { return Config.MaxSafeEngineRpm; }
    uint32 GetEventSerial() const { return EventSerial; }
    float GetLeverX() const { return GateState.LeverX; }
    float GetLeverY() const { return GateState.LeverY; }

    float ComputeClutchCoupling(float ClutchPedal) const;
    float ExpectedEngineRpmForGear(int32 Gear, float SpeedKmh) const;
    bool MoveGate(float X, float Y);
    bool ApplyLeverDriverDelta(float DriverRightCounts, float DriverForwardCounts);
    void CancelPendingRequest();
    bool RequestGear(int32 Gear, const FPinkCabGearEngagementContext& Context);
    bool RequestGearByDelta(int32 Delta, const FPinkCabGearEngagementContext& Context);
    EPinkCabGearEngagementResult EvaluateCurrentEngagement(
        const FPinkCabGearEngagementContext& Context);
    void ForceState(int32 InRequestedGear, int32 InEngagedGear);

private:
    FPinkCabGearboxControllerConfig Config;
    FPinkCabHGateState GateState;
    int32 EngagedGear = 0;
    float ExpectedCoupledRpm = 850.0f;
    uint32 EventSerial = 0;
    EPinkCabGearEngagementResult LastResult = EPinkCabGearEngagementResult::None;
};
