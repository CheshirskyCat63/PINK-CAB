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

struct FPinkCabRoadRuleDefinition
{
    EPinkCabRoadRuleKind Kind = EPinkCabRoadRuleKind::SpeedLimit;
    FName TypeId = NAME_None;
    double SpeedLimitCmPerSec = 0.0;
    int64 FineAmountMinor = 0;
    int32 ReputationDelta = 0;
    float BaseSeverity = 0.0f;

    bool IsValid() const
    {
        if (TypeId == NAME_None || FineAmountMinor < 0
            || BaseSeverity < 0.0f || BaseSeverity > 1.0f)
        {
            return false;
        }
        return Kind != EPinkCabRoadRuleKind::SpeedLimit
            || (FMath::IsFinite(SpeedLimitCmPerSec) && SpeedLimitCmPerSec > 0.0);
    }
};

struct FPinkCabRoadObservation
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

    bool IsValid() const
    {
        return !ObservationKey.TrimStartAndEnd().IsEmpty()
            && LaneId.IsValid() && CityLocationId.IsValid() && VehicleId.IsValid()
            && FMath::IsFinite(SpeedCmPerSec) && SpeedCmPerSec >= 0.0;
    }
};

class FPinkCabRoadRuleProfile
{
public:
    bool TrySetRule(const FPinkCabLaneId& LaneId, const FPinkCabRoadRuleDefinition& Rule)
    {
        if (!LaneId.IsValid() || !Rule.IsValid())
        {
            return false;
        }
        const FString TypeKey = Rule.TypeId.ToString();
        if (const FPinkCabRoadRuleDefinition* ExistingType = RulesByType.Find(TypeKey))
        {
            if (ExistingType->FineAmountMinor != Rule.FineAmountMinor
                || ExistingType->ReputationDelta != Rule.ReputationDelta)
            {
                return false;
            }
        }
        else
        {
            RulesByType.Add(TypeKey, Rule);
        }
        RulesByLaneKind.Add(MakeKey(LaneId, Rule.Kind), Rule);
        return true;
    }

    bool TryGetRule(
        const FPinkCabLaneId& LaneId,
        EPinkCabRoadRuleKind Kind,
        FPinkCabRoadRuleDefinition& OutRule) const
    {
        const FPinkCabRoadRuleDefinition* Found = RulesByLaneKind.Find(MakeKey(LaneId, Kind));
        if (!Found)
        {
            return false;
        }
        OutRule = *Found;
        return true;
    }

    bool TryGetRuleByType(FName TypeId, FPinkCabRoadRuleDefinition& OutRule) const
    {
        const FPinkCabRoadRuleDefinition* Found = RulesByType.Find(TypeId.ToString());
        if (!Found)
        {
            return false;
        }
        OutRule = *Found;
        return true;
    }

private:
    static FString MakeKey(const FPinkCabLaneId& LaneId, EPinkCabRoadRuleKind Kind)
    {
        return FString::Printf(TEXT("%s|%d"), *LaneId.Serialize(), static_cast<int32>(Kind));
    }

    TMap<FString, FPinkCabRoadRuleDefinition> RulesByLaneKind;
    TMap<FString, FPinkCabRoadRuleDefinition> RulesByType;
};
