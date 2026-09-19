#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Persistence/PinkCabSaveHeader.h"

class FPinkCabWorldSessionSnapshotCodec;

class PINKCABPERSISTENCE_API FPinkCabWorkdaySessionState
{
public:
    static bool TryCreate(
        const FPinkCabStableId& InWorkdayId,
        int32 InOrdinal,
        double InElapsedGameSeconds,
        int32 InMaxReplayJournalEntries,
        FPinkCabWorkdaySessionState& OutState);

    const FPinkCabStableId& GetWorkdayId() const;
    int32 GetOrdinal() const;
    double GetElapsedGameSeconds() const;
    bool IsSummaryCommitted() const;
    int32 GetHouseholdTransactionCount() const;
    bool WasTerminalRecovery() const;

    bool AdvanceElapsedSeconds(double DeltaSeconds);
    bool MarkSummaryCommitted();
    bool RecordHouseholdTransactionOnce(const FPinkCabStableId& OperationId);
    bool CanAdvanceToNextWorkday() const;
    void MarkTerminalRecovery();
    bool ResetForNextWorkday(
        const FPinkCabStableId& NextWorkdayId,
        int32 NextOrdinal);

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
