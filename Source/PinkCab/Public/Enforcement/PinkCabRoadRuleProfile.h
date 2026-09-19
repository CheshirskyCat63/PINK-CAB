#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "World/PinkCabRoadGraph.h"

enum class EPinkCabRoadRuleKind : uint8
{
    SpeedLimit,
    LaneRestriction,
    TurnRestriction,
    ParkingRestriction,
    TransitRestriction
};

struct PINKCAB_API FPinkCabRoadRuleDefinition
{
    EPinkCabRoadRuleKind Kind = EPinkCabRoadRuleKind::SpeedLimit;
    FName TypeId = NAME_None;
    double SpeedLimitCmPerSec = 0.0;
    int64 FineAmountMinor = 0;
    int32 ReputationDelta = 0;
    float BaseSeverity = 0.0f;

    bool IsValid() const;
};

struct PINKCAB_API FPinkCabRoadObservation
{
    FString ObservationKey;
    FPinkCabLaneId LaneId;
    FPinkCabStableId CityLocationId;
    FPinkCabStableId VehicleId;
    int64 TimestampMs = 0;
    double SpeedCmPerSec = 0.0;
    bool bLaneViolation = false;
    bool bTurnViolation = false;
    bool bParkingViolation = false;
    bool bTransitViolation = false;

    bool IsValid() const;
};

class PINKCAB_API FPinkCabRoadRuleProfile
{
public:
    bool TrySetRule(
        const FPinkCabLaneId& LaneId,
        const FPinkCabRoadRuleDefinition& Rule);
    bool TryGetRule(
        const FPinkCabLaneId& LaneId,
        EPinkCabRoadRuleKind Kind,
        FPinkCabRoadRuleDefinition& OutRule) const;
    bool TryGetRuleByType(
        FName TypeId,
        FPinkCabRoadRuleDefinition& OutRule) const;

private:
    static FString MakeKey(
        const FPinkCabLaneId& LaneId,
        EPinkCabRoadRuleKind Kind);

    TMap<FString, FPinkCabRoadRuleDefinition> RulesByLaneKind;
    TMap<FString, FPinkCabRoadRuleDefinition> RulesByType;
};
