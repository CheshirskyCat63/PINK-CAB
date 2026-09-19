#include "Enforcement/PinkCabEnforcementLedger.h"

void FPinkCabEnforcementRuleProfile::SetRule(
    FName TypeId,
    const FPinkCabEnforcementRule& Rule)
{
    if (TypeId != NAME_None)
    {
        Rules.Add(TypeId, Rule);
    }
}

bool FPinkCabEnforcementRuleProfile::TryGetRule(
    FName TypeId,
    FPinkCabEnforcementRule& OutRule) const
{
    const FPinkCabEnforcementRule* Found = Rules.Find(TypeId);
    if (!Found) return false;

    OutRule = *Found;
    return true;
}

FPinkCabEnforcementLedger::FPinkCabEnforcementLedger(
    int32 InRecentCapacity)
    : RecentCapacity(FMath::Max(1, InRecentCapacity))
{
}

bool FPinkCabEnforcementLedger::Record(
    const FPinkCabEnforcementEvent& Event,
    int32 ReputationDelta)
{
    if (!Event.IsValid()
        || RecordedEventIds.Contains(Event.EventId.Serialize()))
    {
        return false;
    }

    RecordedEventIds.Add(Event.EventId.Serialize());
    RecentEvents.Add(Event);
    while (RecentEvents.Num() > RecentCapacity)
    {
        RecentEvents.RemoveAt(0);
    }
    ++TotalEventCount;
    ReputationScore += ReputationDelta;
    return true;
}

int32 FPinkCabEnforcementLedger::GetRecentCount() const
{
    return RecentEvents.Num();
}

int32 FPinkCabEnforcementLedger::GetTotalEventCount() const
{
    return TotalEventCount;
}

int32 FPinkCabEnforcementLedger::GetReputationScore() const
{
    return ReputationScore;
}

bool FPinkCabEnforcementLedger::HasRecordedEvent(
    const FPinkCabStableId& EventId) const
{
    return EventId.IsValid()
        && RecordedEventIds.Contains(EventId.Serialize());
}

EPinkCabReputationBand FPinkCabEnforcementLedger::ResolveBand(
    const FPinkCabReputationBandPolicy& Policy) const
{
    if (ReputationScore >= Policy.CleanMinimum)
    {
        return EPinkCabReputationBand::Clean;
    }
    if (ReputationScore >= Policy.WatchMinimum)
    {
        return EPinkCabReputationBand::Watch;
    }
    return EPinkCabReputationBand::Problem;
}
