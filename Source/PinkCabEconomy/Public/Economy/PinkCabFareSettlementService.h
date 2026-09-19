#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Economy/PinkCabEconomyLedger.h"

enum class EPinkCabFareSettlementDisposition : uint8
{
    Committed,
    Replayed,
    Conflict,
    Invalid,
    CapacityExceeded
};

enum class EPinkCabFareSettlementKind : uint8
{
    Paid,
    Evaded
};

struct FPinkCabFareSettlementResult
{
    EPinkCabFareSettlementDisposition Disposition =
        EPinkCabFareSettlementDisposition::Invalid;
    EPinkCabSettlementResult FareResult =
        EPinkCabSettlementResult::InvalidTransaction;
    EPinkCabSettlementResult TipResult =
        EPinkCabSettlementResult::InvalidTransaction;
    FPinkCabStableId ConsequenceId;
};

class PINKCABECONOMY_API FPinkCabFareSettlementService
{
public:
    explicit FPinkCabFareSettlementService(
        int32 InMaxResolvedFares = 512);

    FPinkCabFareSettlementResult CommitPaid(
        const FPinkCabStableId& FareId,
        int64 FareMinor,
        int64 TipMinor,
        FPinkCabEconomyLedger& Ledger);

    FPinkCabFareSettlementResult RecordEvasion(
        const FPinkCabStableId& FareId);

    int32 NumResolvedFares() const;

private:
    struct FPinkCabFareSettlementRecord
    {
        EPinkCabFareSettlementKind Kind =
            EPinkCabFareSettlementKind::Paid;
        EPinkCabSettlementResult FareResult =
            EPinkCabSettlementResult::InvalidTransaction;
        EPinkCabSettlementResult TipResult =
            EPinkCabSettlementResult::InvalidTransaction;
        FPinkCabStableId ConsequenceId;

        FPinkCabFareSettlementResult ToResult(
            EPinkCabFareSettlementDisposition Disposition) const
        {
            return {
                Disposition,
                FareResult,
                TipResult,
                ConsequenceId};
        }

        FPinkCabFareSettlementResult ToReplayResult() const
        {
            return ToResult(
                EPinkCabFareSettlementDisposition::Replayed);
        }
    };
    bool TryResolveExisting(
        const FPinkCabStableId& FareId,
        EPinkCabFareSettlementKind ExpectedKind,
        FPinkCabFareSettlementResult& OutResult) const;
    bool HasRecordCapacity() const;

    static bool IsAcceptedCommitResult(
        EPinkCabSettlementResult Result);
    static TArray<FString> BuildRequiredReplayIds(
        const FPinkCabStableId& FareId,
        int64 TipMinor);
    static EPinkCabSettlementResult CommitTip(
        const FString& TipTxKey,
        int64 TipMinor,
        FPinkCabEconomyLedger& Ledger);

    friend class FPinkCabEconomySnapshotCodec;

    int32 MaxResolvedFares = 512;
    TMap<FString, FPinkCabFareSettlementRecord> Records;
};
