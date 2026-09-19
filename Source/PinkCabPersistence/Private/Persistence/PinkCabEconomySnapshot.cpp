#include "Persistence/PinkCabEconomySnapshot.h"

bool FPinkCabEconomySnapshotCodec::Capture(
    const FPinkCabEconomyLedger& Ledger,
    const FPinkCabFareSettlementService& Settlement,
    FPinkCabEconomySnapshot& OutSnapshot)
{
    FPinkCabEconomySnapshot Snapshot;
    Snapshot.BalanceMinor = Ledger.BalanceMinor;
    Snapshot.DebtLimitMinor = Ledger.DebtLimitMinor;
    Snapshot.MaxCommittedTransactionIds = Ledger.MaxReplayJournalEntries;
    Snapshot.MaxResolvedFares = Settlement.MaxResolvedFares;

    for (const FString& Id : Ledger.CommittedIds)
    {
        Snapshot.CommittedTransactionIds.Add(Id);
    }
    Snapshot.CommittedTransactionIds.Sort();

    for (const auto& Pair : Settlement.Records)
    {
        FPinkCabFareSettlementSnapshotRecord Record;
        Record.FareId = Pair.Key;
        Record.Kind = Pair.Value.Kind;
        Record.FareResult = Pair.Value.FareResult;
        Record.TipResult = Pair.Value.TipResult;
        Record.ConsequenceId = Pair.Value.ConsequenceId.Serialize();
        Snapshot.FareSettlements.Add(MoveTemp(Record));
    }
    Snapshot.FareSettlements.Sort([](
        const FPinkCabFareSettlementSnapshotRecord& A,
        const FPinkCabFareSettlementSnapshotRecord& B)
    {
        return A.FareId < B.FareId;
    });

    if (!Validate(Snapshot))
    {
        return false;
    }

    OutSnapshot = MoveTemp(Snapshot);
    return true;
}

bool FPinkCabEconomySnapshotCodec::Restore(
    const FPinkCabEconomySnapshot& Snapshot,
    FPinkCabEconomyLedger& OutLedger,
    FPinkCabFareSettlementService& OutSettlement)
{
    if (!Validate(Snapshot))
    {
        return false;
    }

    FPinkCabEconomyLedger Ledger(
        Snapshot.BalanceMinor,
        Snapshot.DebtLimitMinor,
        Snapshot.MaxCommittedTransactionIds);
    for (const FString& Id : Snapshot.CommittedTransactionIds)
    {
        Ledger.CommittedIds.Add(Id);
    }

    FPinkCabFareSettlementService Settlement(
        Snapshot.MaxResolvedFares);
    for (const FPinkCabFareSettlementSnapshotRecord& Saved
        : Snapshot.FareSettlements)
    {
        FPinkCabFareSettlementService::FPinkCabFareSettlementRecord Record;
        Record.Kind = Saved.Kind;
        Record.FareResult = Saved.FareResult;
        Record.TipResult = Saved.TipResult;
        if (!Saved.ConsequenceId.IsEmpty())
        {
            Record.ConsequenceId = FPinkCabStableId(Saved.ConsequenceId);
        }
        Settlement.Records.Add(Saved.FareId, MoveTemp(Record));
    }

    OutLedger = MoveTemp(Ledger);
    OutSettlement = MoveTemp(Settlement);
    return true;
}

bool FPinkCabEconomySnapshotCodec::ValidateStableIds(
    const TArray<FString>& Ids,
    int32 Capacity)
{
    if (Capacity <= 0 || Ids.Num() > Capacity)
    {
        return false;
    }

    TSet<FString> Unique;
    for (const FString& Id : Ids)
    {
        FPinkCabStableId Parsed;
        if (!FPinkCabStableId::TryParse(Id, Parsed))
        {
            return false;
        }
        if (Unique.Contains(Parsed.Serialize()))
        {
            return false;
        }
        Unique.Add(Parsed.Serialize());
    }
    return true;
}

bool FPinkCabEconomySnapshotCodec::ValidateSettlementResult(
    EPinkCabSettlementResult Result)
{
    return static_cast<uint8>(Result)
        <= static_cast<uint8>(EPinkCabSettlementResult::CapacityExceeded);
}

bool FPinkCabEconomySnapshotCodec::IsAcceptedPaidResult(
    EPinkCabSettlementResult Result)
{
    return Result == EPinkCabSettlementResult::Committed
        || Result == EPinkCabSettlementResult::Duplicate;
}

bool FPinkCabEconomySnapshotCodec::ValidatePaidRecord(
    const FPinkCabFareSettlementSnapshotRecord& Record)
{
    return IsAcceptedPaidResult(Record.FareResult)
        && IsAcceptedPaidResult(Record.TipResult)
        && Record.ConsequenceId.IsEmpty();
}

bool FPinkCabEconomySnapshotCodec::ValidateEvadedRecord(
    const FPinkCabFareSettlementSnapshotRecord& Record)
{
    return Record.ConsequenceId == Record.FareId + TEXT(".evasion");
}

bool FPinkCabEconomySnapshotCodec::ValidateRecord(
    const FPinkCabFareSettlementSnapshotRecord& Record)
{
    if (!ValidateSettlementResult(Record.FareResult)
        || !ValidateSettlementResult(Record.TipResult))
    {
        return false;
    }

    switch (Record.Kind)
    {
    case EPinkCabFareSettlementKind::Paid:
        return ValidatePaidRecord(Record);
    case EPinkCabFareSettlementKind::Evaded:
        return ValidateEvadedRecord(Record);
    default:
        return false;
    }
}

bool FPinkCabEconomySnapshotCodec::ValidateSnapshotHeader(
    const FPinkCabEconomySnapshot& Snapshot)
{
    return Snapshot.SchemaVersion
            == FPinkCabEconomySnapshot::CurrentSchemaVersion
        && Snapshot.DebtLimitMinor >= 0
        && ValidateStableIds(
            Snapshot.CommittedTransactionIds,
            Snapshot.MaxCommittedTransactionIds)
        && Snapshot.MaxResolvedFares > 0
        && Snapshot.FareSettlements.Num()
            <= Snapshot.MaxResolvedFares;
}

bool FPinkCabEconomySnapshotCodec::Validate(
    const FPinkCabEconomySnapshot& Snapshot)
{
    if (!ValidateSnapshotHeader(Snapshot))
    {
        return false;
    }

    TSet<FString> FareIds;
    for (const FPinkCabFareSettlementSnapshotRecord& Record
        : Snapshot.FareSettlements)
    {
        FPinkCabStableId FareId;
        if (!FPinkCabStableId::TryParse(Record.FareId, FareId))
        {
            return false;
        }
        if (FareIds.Contains(FareId.Serialize()))
        {
            return false;
        }
        if (!ValidateRecord(Record))
        {
            return false;
        }
        FareIds.Add(FareId.Serialize());
    }
    return true;
}
