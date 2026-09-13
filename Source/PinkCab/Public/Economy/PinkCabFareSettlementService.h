#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Economy/PinkCabEconomyLedger.h"

enum class EPinkCabFareSettlementDisposition : uint8
{
    Committed,
    Replayed,
    Conflict,
    Invalid
};

enum class EPinkCabFareSettlementKind : uint8
{
    Paid,
    Evaded
};

struct FPinkCabFareSettlementResult
{
    EPinkCabFareSettlementDisposition Disposition = EPinkCabFareSettlementDisposition::Invalid;
    EPinkCabSettlementResult FareResult = EPinkCabSettlementResult::InvalidTransaction;
    EPinkCabSettlementResult TipResult = EPinkCabSettlementResult::InvalidTransaction;
    FPinkCabStableId ConsequenceId;
};

class FPinkCabFareSettlementService
{
public:
    FPinkCabFareSettlementResult CommitPaid(
        const FPinkCabStableId& FareId,
        const int64 FareMinor,
        const int64 TipMinor,
        FPinkCabEconomyLedger& Ledger)
    {
        if (!FareId.IsValid() || FareMinor <= 0 || TipMinor < 0)
        {
            return {};
        }

        if (const FPinkCabFareSettlementRecord* Existing = Records.Find(FareId.Serialize()))
        {
            if (Existing->Kind != EPinkCabFareSettlementKind::Paid)
            {
                return {EPinkCabFareSettlementDisposition::Conflict};
            }
            return Existing->ToReplayResult();
        }

        const FPinkCabTransactionId FareTxId(FareId.Serialize() + TEXT(".fare"));
        const EPinkCabSettlementResult FareResult = Ledger.Commit(
            FPinkCabEconomyTransaction::Credit(FareTxId, EPinkCabTransactionType::FareIncome, FareMinor));

        if (FareResult != EPinkCabSettlementResult::Committed
            && FareResult != EPinkCabSettlementResult::Duplicate)
        {
            return {EPinkCabFareSettlementDisposition::Invalid, FareResult};
        }

        EPinkCabSettlementResult TipResult = EPinkCabSettlementResult::Committed;
        if (TipMinor > 0)
        {
            const FPinkCabTransactionId TipTxId(FareId.Serialize() + TEXT(".tip"));
            TipResult = Ledger.Commit(
                FPinkCabEconomyTransaction::Credit(TipTxId, EPinkCabTransactionType::Tip, TipMinor));
            if (TipResult != EPinkCabSettlementResult::Committed
                && TipResult != EPinkCabSettlementResult::Duplicate)
            {
                return {EPinkCabFareSettlementDisposition::Invalid, FareResult, TipResult};
            }
        }

        FPinkCabFareSettlementRecord Record;
        Record.Kind = EPinkCabFareSettlementKind::Paid;
        Record.FareResult = FareResult;
        Record.TipResult = TipResult;
        Records.Add(FareId.Serialize(), Record);
        const bool bRecoveredReplay = FareResult == EPinkCabSettlementResult::Duplicate
            || TipResult == EPinkCabSettlementResult::Duplicate;
        return Record.ToResult(bRecoveredReplay
            ? EPinkCabFareSettlementDisposition::Replayed
            : EPinkCabFareSettlementDisposition::Committed);
    }

    FPinkCabFareSettlementResult RecordEvasion(const FPinkCabStableId& FareId)
    {
        if (!FareId.IsValid())
        {
            return {};
        }

        if (const FPinkCabFareSettlementRecord* Existing = Records.Find(FareId.Serialize()))
        {
            if (Existing->Kind != EPinkCabFareSettlementKind::Evaded)
            {
                return {EPinkCabFareSettlementDisposition::Conflict};
            }
            return Existing->ToReplayResult();
        }

        FPinkCabFareSettlementRecord Record;
        Record.Kind = EPinkCabFareSettlementKind::Evaded;
        Record.ConsequenceId = FPinkCabStableId(FareId.Serialize() + TEXT(".evasion"));
        Records.Add(FareId.Serialize(), Record);
        return Record.ToResult(EPinkCabFareSettlementDisposition::Committed);
    }

    int32 NumResolvedFares() const
    {
        return Records.Num();
    }

private:
    struct FPinkCabFareSettlementRecord
    {
        EPinkCabFareSettlementKind Kind = EPinkCabFareSettlementKind::Paid;
        EPinkCabSettlementResult FareResult = EPinkCabSettlementResult::InvalidTransaction;
        EPinkCabSettlementResult TipResult = EPinkCabSettlementResult::InvalidTransaction;
        FPinkCabStableId ConsequenceId;

        FPinkCabFareSettlementResult ToResult(const EPinkCabFareSettlementDisposition Disposition) const
        {
            return {Disposition, FareResult, TipResult, ConsequenceId};
        }

        FPinkCabFareSettlementResult ToReplayResult() const
        {
            return ToResult(EPinkCabFareSettlementDisposition::Replayed);
        }
    };

    TMap<FString, FPinkCabFareSettlementRecord> Records;
};

