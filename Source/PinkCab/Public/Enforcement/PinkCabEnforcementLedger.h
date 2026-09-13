#pragma once

#include "CoreMinimal.h"
#include "Enforcement/PinkCabEnforcementEvent.h"

enum class EPinkCabReputationBand : uint8
{
    Clean,
    Watch,
    Problem
};

struct FPinkCabReputationBandPolicy
{
    int32 CleanMinimum = 0;
    int32 WatchMinimum = 0;
};

struct FPinkCabEnforcementRule
{
    int64 FineAmountMinor = 0;
    int32 ReputationDelta = 0;
};

class FPinkCabEnforcementRuleProfile
{
public:
    void SetRule(FName TypeId, const FPinkCabEnforcementRule& Rule)
    {
        if (TypeId != NAME_None) Rules.Add(TypeId, Rule);
    }
    bool TryGetRule(FName TypeId, FPinkCabEnforcementRule& OutRule) const
    {
        const FPinkCabEnforcementRule* Found = Rules.Find(TypeId);
        if (!Found) return false;
        OutRule = *Found;
        return true;
    }

private:
    TMap<FName, FPinkCabEnforcementRule> Rules;
};

class FPinkCabEnforcementLedger
{
public:
    explicit FPinkCabEnforcementLedger(int32 InRecentCapacity)
        : RecentCapacity(FMath::Max(1, InRecentCapacity))
    {
    }

    bool Record(const FPinkCabEnforcementEvent& Event, int32 ReputationDelta)
    {
        if (!Event.IsValid() || RecordedEventIds.Contains(Event.EventId.Serialize())) return false;
        RecordedEventIds.Add(Event.EventId.Serialize());
        RecentEvents.Add(Event);
        while (RecentEvents.Num() > RecentCapacity) RecentEvents.RemoveAt(0);
        ++TotalEventCount;
        ReputationScore += ReputationDelta;
        return true;
    }
    int32 GetRecentCount() const { return RecentEvents.Num(); }
    int32 GetTotalEventCount() const { return TotalEventCount; }
    int32 GetReputationScore() const { return ReputationScore; }
    bool HasRecordedEvent(const FPinkCabStableId& EventId) const
    {
        return EventId.IsValid() && RecordedEventIds.Contains(EventId.Serialize());
    }

    EPinkCabReputationBand ResolveBand(const FPinkCabReputationBandPolicy& Policy) const
    {
        if (ReputationScore >= Policy.CleanMinimum) return EPinkCabReputationBand::Clean;
        if (ReputationScore >= Policy.WatchMinimum) return EPinkCabReputationBand::Watch;
        return EPinkCabReputationBand::Problem;
    }

private:
    int32 RecentCapacity = 1;
    int32 TotalEventCount = 0;
    int32 ReputationScore = 0;
    TArray<FPinkCabEnforcementEvent> RecentEvents;
    TSet<FString> RecordedEventIds;
};
