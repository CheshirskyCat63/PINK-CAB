#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Persistence/PinkCabSaveHeader.h"

class FPinkCabWorldSessionSnapshotCodec;

class FPinkCabWorkdaySessionState
{
public:
    static bool TryCreate(
        const FPinkCabStableId& InWorkdayId,
        int32 InOrdinal,
        double InElapsedGameSeconds,
        int32 InMaxReplayJournalEntries,
        FPinkCabWorkdaySessionState& OutState)
    {
        if (!InWorkdayId.IsValid() || InOrdinal < 0
            || !FMath::IsFinite(InElapsedGameSeconds) || InElapsedGameSeconds < 0.0
            || InMaxReplayJournalEntries <= 0)
        {
            return false;
        }

        FPinkCabWorkdaySessionState State;
        State.WorkdayId = InWorkdayId;
        State.Ordinal = InOrdinal;
        State.ElapsedGameSeconds = InElapsedGameSeconds;
        State.MaxReplayJournalEntries = InMaxReplayJournalEntries;
        OutState = MoveTemp(State);
        return true;
    }

    const FPinkCabStableId& GetWorkdayId() const { return WorkdayId; }
    int32 GetOrdinal() const { return Ordinal; }
    double GetElapsedGameSeconds() const { return ElapsedGameSeconds; }
    bool IsSummaryCommitted() const { return bSummaryCommitted; }
    int32 GetHouseholdTransactionCount() const { return HouseholdTransactionCount; }
    bool WasTerminalRecovery() const { return bTerminalRecovery; }

    bool AdvanceElapsedSeconds(double DeltaSeconds)
    {
        if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.0 || !WorkdayId.IsValid())
        {
            return false;
        }
        ElapsedGameSeconds += DeltaSeconds;
        return FMath::IsFinite(ElapsedGameSeconds);
    }

    bool MarkSummaryCommitted()
    {
        if (!WorkdayId.IsValid() || bSummaryCommitted)
        {
            return false;
        }
        bSummaryCommitted = true;
        return true;
    }

    bool RecordHouseholdTransactionOnce(const FPinkCabStableId& OperationId)
    {
        if (!WorkdayId.IsValid() || !OperationId.IsValid())
        {
            return false;
        }
        const FString Key = OperationId.Serialize();
        if (AppliedHouseholdOperationIds.Contains(Key)
            || AppliedHouseholdOperationIds.Num() >= MaxReplayJournalEntries)
        {
            return false;
        }
        AppliedHouseholdOperationIds.Add(Key);
        ++HouseholdTransactionCount;
        return true;
    }

    bool CanAdvanceToNextWorkday() const
    {
        return FPinkCabPersistencePolicy::CanAdvanceToNextWorkday(
            bSummaryCommitted, HouseholdTransactionCount);
    }

    void MarkTerminalRecovery()
    {
        bTerminalRecovery = true;
    }

    bool ResetForNextWorkday(const FPinkCabStableId& NextWorkdayId, int32 NextOrdinal)
    {
        if (!CanAdvanceToNextWorkday() || !NextWorkdayId.IsValid() || NextOrdinal <= Ordinal)
        {
            return false;
        }
        WorkdayId = NextWorkdayId;
        Ordinal = NextOrdinal;
        ElapsedGameSeconds = 0.0;
        bSummaryCommitted = false;
        HouseholdTransactionCount = 0;
        bTerminalRecovery = false;
        AppliedHouseholdOperationIds.Reset();
        return true;
    }

private:
    friend class FPinkCabWorldSessionSnapshotCodec;

    FPinkCabStableId WorkdayId;
    int32 Ordinal = 0;
    double ElapsedGameSeconds = 0.0;
    int32 MaxReplayJournalEntries = 64;
    bool bSummaryCommitted = false;
    int32 HouseholdTransactionCount = 0;
    bool bTerminalRecovery = false;
    TSet<FString> AppliedHouseholdOperationIds;
};
