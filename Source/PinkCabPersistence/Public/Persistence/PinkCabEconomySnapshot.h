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

class PINKCABPERSISTENCE_API FPinkCabEconomySnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabEconomyLedger& Ledger,
        const FPinkCabFareSettlementService& Settlement,
        FPinkCabEconomySnapshot& OutSnapshot);
    static bool Restore(
        const FPinkCabEconomySnapshot& Snapshot,
        FPinkCabEconomyLedger& OutLedger,
        FPinkCabFareSettlementService& OutSettlement);

private:
    static bool ValidateStableIds(
        const TArray<FString>& Ids,
        int32 Capacity);
    static bool ValidateSettlementResult(
        EPinkCabSettlementResult Result);
    static bool IsAcceptedPaidResult(
        EPinkCabSettlementResult Result);
    static bool ValidatePaidRecord(
        const FPinkCabFareSettlementSnapshotRecord& Record);
    static bool ValidateEvadedRecord(
        const FPinkCabFareSettlementSnapshotRecord& Record);
    static bool ValidateRecord(
        const FPinkCabFareSettlementSnapshotRecord& Record);
    static bool ValidateSnapshotHeader(
        const FPinkCabEconomySnapshot& Snapshot);
    static bool Validate(
        const FPinkCabEconomySnapshot& Snapshot);
};
