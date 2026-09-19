#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Economy/PinkCabFareSettlementService.h"

struct FPinkCabFareSettlementSnapshotRecord
{
    FString FareId;
    EPinkCabFareSettlementKind Kind = EPinkCabFareSettlementKind::Paid;
    EPinkCabSettlementResult FareResult = EPinkCabSettlementResult::InvalidTransaction;
    EPinkCabSettlementResult TipResult = EPinkCabSettlementResult::InvalidTransaction;
    FString ConsequenceId;
};

struct FPinkCabEconomySnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;

    int32 SchemaVersion = CurrentSchemaVersion;
    int64 BalanceMinor = 0;
    int64 DebtLimitMinor = 0;
    int32 MaxCommittedTransactionIds = 0;
    int32 MaxResolvedFares = 0;
    TArray<FString> CommittedTransactionIds;
    TArray<FPinkCabFareSettlementSnapshotRecord> FareSettlements;
};

class FPinkCabEconomySnapshotCodec
{
public:
    static bool Capture(
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
        }        Snapshot.FareSettlements.Sort([](const auto& A, const auto& B)
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

    static bool Restore(
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

        FPinkCabFareSettlementService Settlement(Snapshot.MaxResolvedFares);        for (const FPinkCabFareSettlementSnapshotRecord& Saved : Snapshot.FareSettlements)
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

private:
    static bool ValidateStableIds(const TArray<FString>& Ids, int32 Capacity)
    {
        if (Capacity <= 0 || Ids.Num() > Capacity)
        {
            return false;
        }
        TSet<FString> Unique;
        for (const FString& Id : Ids)
        {
            FPinkCabStableId Parsed;
            if (!FPinkCabStableId::TryParse(Id, Parsed) || Unique.Contains(Parsed.Serialize()))
            {
                return false;
            }            Unique.Add(Parsed.Serialize());
        }
        return true;
    }

    static bool ValidateSettlementResult(EPinkCabSettlementResult Result)
    {
        return static_cast<uint8>(Result)
            <= static_cast<uint8>(EPinkCabSettlementResult::CapacityExceeded);
    }

    static bool Validate(const FPinkCabEconomySnapshot& Snapshot)
    {
        if (Snapshot.SchemaVersion != FPinkCabEconomySnapshot::CurrentSchemaVersion
            || Snapshot.DebtLimitMinor < 0
            || !ValidateStableIds(
                Snapshot.CommittedTransactionIds,
                Snapshot.MaxCommittedTransactionIds)
            || Snapshot.MaxResolvedFares <= 0
            || Snapshot.FareSettlements.Num() > Snapshot.MaxResolvedFares)
        {
            return false;
        }

        TSet<FString> FareIds;
        for (const FPinkCabFareSettlementSnapshotRecord& Record : Snapshot.FareSettlements)
        {
            FPinkCabStableId FareId;
            if (!FPinkCabStableId::TryParse(Record.FareId, FareId)
                || FareIds.Contains(FareId.Serialize())
                || !ValidateSettlementResult(Record.FareResult)
                || !ValidateSettlementResult(Record.TipResult))
            {
                return false;
            }            FareIds.Add(FareId.Serialize());

            if (Record.Kind == EPinkCabFareSettlementKind::Paid)
            {
                const bool bFareOk = Record.FareResult == EPinkCabSettlementResult::Committed
                    || Record.FareResult == EPinkCabSettlementResult::Duplicate;
                const bool bTipOk = Record.TipResult == EPinkCabSettlementResult::Committed
                    || Record.TipResult == EPinkCabSettlementResult::Duplicate;
                if (!bFareOk || !bTipOk || !Record.ConsequenceId.IsEmpty())
                {
                    return false;
                }
            }
            else if (Record.Kind == EPinkCabFareSettlementKind::Evaded)
            {
                if (Record.ConsequenceId != Record.FareId + TEXT(".evasion"))
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        return true;
    }
};
