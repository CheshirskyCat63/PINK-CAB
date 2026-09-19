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

class PINKCAB_API FPinkCabEnforcementRuleProfile
{
public:
    void SetRule(
        FName TypeId,
        const FPinkCabEnforcementRule& Rule);
    bool TryGetRule(
        FName TypeId,
        FPinkCabEnforcementRule& OutRule) const;

private:
    TMap<FName, FPinkCabEnforcementRule> Rules;
};

class PINKCAB_API FPinkCabEnforcementLedger
{
public:
    explicit FPinkCabEnforcementLedger(int32 InRecentCapacity);

    bool Record(
        const FPinkCabEnforcementEvent& Event,
        int32 ReputationDelta);
    int32 GetRecentCount() const;
    int32 GetTotalEventCount() const;
    int32 GetReputationScore() const;
    bool HasRecordedEvent(
        const FPinkCabStableId& EventId) const;
    EPinkCabReputationBand ResolveBand(
        const FPinkCabReputationBandPolicy& Policy) const;

private:
    int32 RecentCapacity = 1;
    int32 TotalEventCount = 0;
    int32 ReputationScore = 0;
    TArray<FPinkCabEnforcementEvent> RecentEvents;
    TSet<FString> RecordedEventIds;
};
