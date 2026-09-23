#include "Persistence/PinkCabWorkdaySessionState.h"

bool FPinkCabWorkdaySessionState::TryCreate(
    const FPinkCabStableId& InWorkdayId,
    int32 InOrdinal,
    double InElapsedGameSeconds,
    int32 InMaxReplayJournalEntries,
    FPinkCabWorkdaySessionState& OutState)
{
    if (!InWorkdayId.IsValid()
        || InOrdinal < 0
        || !FMath::IsFinite(InElapsedGameSeconds)
        || InElapsedGameSeconds < 0.0
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

const FPinkCabStableId& FPinkCabWorkdaySessionState::GetWorkdayId() const { return WorkdayId; }
int32 FPinkCabWorkdaySessionState::GetOrdinal() const { return Ordinal; }
double FPinkCabWorkdaySessionState::GetElapsedGameSeconds() const { return ElapsedGameSeconds; }
bool FPinkCabWorkdaySessionState::IsSummaryCommitted() const { return bSummaryCommitted; }
int32 FPinkCabWorkdaySessionState::GetHouseholdTransactionCount() const { return HouseholdTransactionCount; }
bool FPinkCabWorkdaySessionState::WasTerminalRecovery() const { return bTerminalRecovery; }

bool FPinkCabWorkdaySessionState::AdvanceElapsedSeconds(double DeltaSeconds)
{
    if (!FMath::IsFinite(DeltaSeconds)
        || DeltaSeconds <= 0.0
        || !WorkdayId.IsValid())
    {
        return false;
    }
    ElapsedGameSeconds += DeltaSeconds;
    return FMath::IsFinite(ElapsedGameSeconds);
}

bool FPinkCabWorkdaySessionState::MarkSummaryCommitted()
{
    if (!WorkdayId.IsValid() || bSummaryCommitted)
    {
        return false;
    }
    bSummaryCommitted = true;
    return true;
}

bool FPinkCabWorkdaySessionState::RecordHouseholdTransactionOnce(
    const FPinkCabStableId& OperationId)
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

bool FPinkCabWorkdaySessionState::CanAdvanceToNextWorkday() const
{
    return FPinkCabPersistencePolicy::CanAdvanceToNextWorkday(
        bSummaryCommitted,
        HouseholdTransactionCount);
}

void FPinkCabWorkdaySessionState::MarkTerminalRecovery()
{
    bTerminalRecovery = true;
}

bool FPinkCabWorkdaySessionState::ResetForNextWorkday(
    const FPinkCabStableId& NextWorkdayId,
    int32 NextOrdinal)
{
    if (!CanAdvanceToNextWorkday()
        || !NextWorkdayId.IsValid()
        || NextOrdinal <= Ordinal)
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
