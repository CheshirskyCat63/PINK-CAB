#include "Enforcement/PinkCabRoadRuleProfile.h"

bool FPinkCabRoadRuleDefinition::IsValid() const
{
    if (TypeId == NAME_None
        || FineAmountMinor < 0
        || BaseSeverity < 0.0f
        || BaseSeverity > 1.0f)
    {
        return false;
    }

    return Kind != EPinkCabRoadRuleKind::SpeedLimit
        || (FMath::IsFinite(SpeedLimitCmPerSec)
            && SpeedLimitCmPerSec > 0.0);
}

bool FPinkCabRoadObservation::IsValid() const
{
    return !ObservationKey.TrimStartAndEnd().IsEmpty()
        && LaneId.IsValid()
        && CityLocationId.IsValid()
        && VehicleId.IsValid()
        && FMath::IsFinite(SpeedCmPerSec)
        && SpeedCmPerSec >= 0.0;
}

bool FPinkCabRoadRuleProfile::TrySetRule(
    const FPinkCabLaneId& LaneId,
    const FPinkCabRoadRuleDefinition& Rule)
{
    if (!LaneId.IsValid() || !Rule.IsValid())
    {
        return false;
    }

    const FString TypeKey = Rule.TypeId.ToString();
    if (const FPinkCabRoadRuleDefinition* ExistingType =
        RulesByType.Find(TypeKey))
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

bool FPinkCabRoadRuleProfile::TryGetRule(
    const FPinkCabLaneId& LaneId,
    EPinkCabRoadRuleKind Kind,
    FPinkCabRoadRuleDefinition& OutRule) const
{
    const FPinkCabRoadRuleDefinition* Found =
        RulesByLaneKind.Find(MakeKey(LaneId, Kind));
    if (!Found) return false;

    OutRule = *Found;
    return true;
}

bool FPinkCabRoadRuleProfile::TryGetRuleByType(
    FName TypeId,
    FPinkCabRoadRuleDefinition& OutRule) const
{
    const FPinkCabRoadRuleDefinition* Found =
        RulesByType.Find(TypeId.ToString());
    if (!Found) return false;

    OutRule = *Found;
    return true;
}

FString FPinkCabRoadRuleProfile::MakeKey(
    const FPinkCabLaneId& LaneId,
    EPinkCabRoadRuleKind Kind)
{
    return FString::Printf(
        TEXT("%s|%d"),
        *LaneId.Serialize(),
        static_cast<int32>(Kind));
}
